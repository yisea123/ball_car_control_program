/**
  ******************************************************************************
  * @file    fatfs_storage.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    11-April-2014
  * @brief   This file includes the Storage (FatsFs) driver 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>

/** @addtogroup STM32_Nucleo_Demo
* @{
*/

/** @defgroup STORAGE
* @brief This file includes the Storage (FatFs) driver for the STM32 Nucleo demo
* @{
*/

/** @defgroup STORAGE_Private_Types
* @{
*/
/**
* @}
*/

/** @defgroup STORAGE_Private_Defines
* @{
*/
/**
* @}
*/

/** @defgroup STORAGE_Private_Macros
* @{
*/
/**
* @}
*/

/** @defgroup STORAGE_Private_Variables
* @{
*/

uint8_t aBuffer[512];
FILINFO MyFileInfo;
DIR MyDirectory;
FIL MyFile;
UINT BytesWritten;
UINT BytesRead;

/**
* @}
*/


/** @defgroup STORAGE_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/

/** @defgroup STORAGE_Private_Functions
* @{
*/


/**
* @brief  Open a file and copy its content to a buffer
* @param  DirName: the Directory name to open
* @param  FileName: the file name to open
* @param  BufferAddress: A pointer to a buffer to copy the file to
* @param  FileLen: the File length
* @retval err: Error status (0=> success, 1=> fail)
*/
uint32_t Storage_OpenReadFile(uint8_t Xpoz, uint16_t Ypoz, const char* BmpName)
{
  uint32_t index = 0, size = 0, width = 0, height = 0, i1 = 0, x = 0, y = 159;
  uint32_t bmpaddress, bit_pixel = 0;
  FIL file1;
  uint16_t color = 0;
  
  f_open(&file1, BmpName, FA_READ);
  f_read(&file1, aBuffer, 30, &BytesRead);
  
  bmpaddress = (uint32_t)aBuffer;
  
  /* Read bitmap size */
  size = *(uint16_t *) (bmpaddress + 2);
  size |= (*(uint16_t *) (bmpaddress + 4)) << 16;
  
  /* Get bitmap data address offset */
  index = *(uint16_t *) (bmpaddress + 10);
  index |= (*(uint16_t *) (bmpaddress + 12)) << 16;
  
  /* Read bitmap width */
  width = *(uint16_t *) (bmpaddress + 18);
  width |= (*(uint16_t *) (bmpaddress + 20)) << 16;
  
  /* Read bitmap height */
  height = *(uint16_t *) (bmpaddress + 22);
  height |= (*(uint16_t *) (bmpaddress + 24)) << 16;
  
  /* Read bit/pixel */
  bit_pixel = *(uint16_t *) (bmpaddress + 28);  
  bit_pixel = bit_pixel/8;
  
  size = (size - index);
  
  /* Synchronize f_read right in front of the image data */
  f_close (&file1);  
 
  f_open (&file1, (TCHAR const*)BmpName, FA_READ);
  
  f_read(&file1, aBuffer, index, &BytesRead);
  
  do
  {
    if(size*2 < 512)
    {
      i1 = size * 2;
    }
    else
    {
      i1 = 512;
    }
    size -= i1; 
   
    f_read(&file1, aBuffer, i1, (UINT *)&BytesRead);
    
    for (index = 0; index < (i1/bit_pixel); index++)
    {
      
      if( index%bit_pixel == 0)
      {
        color = (aBuffer[ index + 1] << 8) | aBuffer[ index];
      }
      x = index/bit_pixel;
      LCD_WriteRAM(x, y, color); 
    }
    y--;
    for (index = (i1/bit_pixel); index < i1 ; index++)
    {
      if( index%bit_pixel == 0)
      {
        color = (aBuffer[ index + 1] << 8) | aBuffer[ index];
      } 
      x = index/bit_pixel - width;
      LCD_WriteRAM(x, y, color); 
    }
    y--;
  }
  while (size > 0);
  f_close(&file1);
  
  return 1;
}      

/**
* @brief  Copy file BmpName1 to BmpName2 
* @param  BmpName1: the source file name
* @param  BmpName2: the destination file name
* @retval err: Error status (0=> success, 1=> fail)
*/
uint32_t Storage_CopyFile(const char* BmpName1, const char* BmpName2)
{
  uint32_t index = 0;
  FIL file1, file2;
  
  /* Open an Existent BMP file system */
  f_open(&file1, BmpName1, FA_READ);
  /* Create a new BMP file system */
  f_open(&file2, BmpName2, FA_CREATE_ALWAYS | FA_WRITE);
  
  do 
  {
    f_read(&file1, aBuffer, _MAX_SS, &BytesRead);
    f_write(&file2, aBuffer, _MAX_SS, &BytesWritten);  
    index+= _MAX_SS;
    
  } while(index < file1.fsize);
  
  f_close(&file1);
  f_close(&file2);
  
  return 1;
}

/**
* @brief  Opens a file and copies its content to a buffer.
* @param  DirName: the Directory name to open
* @param  FileName: the file name to open
* @param  BufferAddress: A pointer to a buffer to copy the file to
* @param  FileLen: File length
* @retval err: Error status (0=> success, 1=> fail)
*/
uint32_t Storage_CheckBitmapFile(const char* BmpName, uint32_t *FileLen)
{
  uint32_t err = 0;
  if(SD_GetStatus() != 0xFF) 
  {
    err = 1;
  }  
  else if(f_open(&MyFile, BmpName, FA_READ) != FR_OK)
  {
    err = 2;
  }
    
  return err;
}

/**
* @brief  List up to 25 file on the root directory with extension .BMP
* @param  DirName: Directory name
* @param  Files: Buffer to contain read files
* @retval The number of the found files
*/
uint32_t Storage_GetDirectoryBitmapFiles(const char* DirName, char* Files[])
{
  uint32_t i = 0, j = 0;
  FRESULT res;
   
  res = f_opendir(&MyDirectory, DirName);
  
  if(res == FR_OK)
  {
    i = strlen(DirName);
    for (;;)
    {
      res = f_readdir(&MyDirectory, &MyFileInfo);
      if(res != FR_OK || MyFileInfo.fname[0] == 0) break;
      if(MyFileInfo.fname[0] == '.') continue;
      
      if(!(MyFileInfo.fattrib & AM_DIR))
      {
        do
        {
          i++;
        }
        while (MyFileInfo.fname[i] != 0x2E);
        
        
        if(j < MAX_BMP_FILES)
        {
          if((MyFileInfo.fname[i + 1] == 'B') && (MyFileInfo.fname[i + 2] == 'M') && (MyFileInfo.fname[i + 3] == 'P'))
          {
            sprintf(Files[j], "%-11.11s", MyFileInfo.fname);
            j++;
          }
        }
        i = 0;
      }
    }
  }
    
  return j;
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared
  * @param  BufferLength: buffer's length
  * @retval  0: pBuffer1 identical to pBuffer2
  *          1: pBuffer1 differs from pBuffer2
  */
uint8_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  uint8_t ret = 1;
  while (BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      ret = 0;
    }
    
    pBuffer1++;
    pBuffer2++;
  }
  
  return ret;
}

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
