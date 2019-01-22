#include <xparameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xil_printf.h>
#include <xtmrctr.h>
#include <xmutex.h>

#define XPAR_MUTEX_0 0
#define XPAR_MUTEX_1 1

XTmrCtr TimerCounterInst;
u32 *dram = ((u32 *)XPAR_MIG7SERIES_0_BASEADDR + 0x300000);//開始信号となる値のアドレス


/*原画像の保存領域 0x80000000 ~ */
/*処理対象の画素情報 0x80100000 ~ */
/*平滑化処理領域2 0x80200000 ~ */
/*平滑化処理領域3 0x80300000 ~ */
/*平滑化処理領域4 0x80400000 ~ */

unsigned char *OriginalPicPtr = ((u8 *)XPAR_MIG7SERIES_0_BASEADDR);
unsigned char *dramPtr = ((u8 *)XPAR_MIG7SERIES_0_BASEADDR + 0x100000);
unsigned char *inIMG = ((u8 *)XPAR_MIG7SERIES_0_BASEADDR + 0x100000);//処理前の画素情報を格納
unsigned char *outIMG = ((u8 *)XPAR_MIG7SERIES_0_BASEADDR + 0x200000);//処理後の画素情報を格納

unsigned char BitMapFileHeader[14]; //BMPのファイルヘッダーを保存する
unsigned int biSize; //BMPのサイズを保存する
int biWidth; //BMPの幅を保存する
int biHeight; //BMPの高さを保存する
unsigned char BitMapInfoHeader[28]; //上記３つ以外のBMPの情報ヘッダーを保存する

void Show_Time(u32, u32);
void Mutex_Lock(XMutex *,u8);
void Mutex_GetStatus(XMutex *,u8,u32 *,u32 *);
int Mutex_Trylock(XMutex *,u8);
int Mutex_Unlock(XMutex *,u8);
int Mutex_IsLocked(XMutex *,u8);
int Mutex_GetUser(XMutex *,u8,u32 *);
int Mutex_SetUser(XMutex *,u8,u32);
void wr_dram();
void rd_dram();
int init();
void Once();
void ReadImageHeader();
void ReadImageFile();
void WriteImageHeader();
int Smooth(XMutex);


//int init();

int main()
{
	//u32 tStart,tEnd;
	XMutex XMutex[2];
	XMutex_Config *InsPtr[2];
	u32 xmutex_status,User;
	int flag=0,endflag=0,i;

	xil_printf("\r\n MB i0 Start");

	ReadImageHeader();
	/*Once();
	Smooth(XMutex[0]);
	WriteImageHeader();

	for(i=0;i<1;i++){
		ReadImageFile();
		Smooth(XMutex[0]);
	}

	WriteImageHeader();*/

	//init();//TimerInitialize

    //-- mutex 0
	InsPtr[0] = XMutex_LookupConfig(XPAR_MUTEX_0_IF_0_DEVICE_ID);

	xmutex_status = XMutex_CfgInitialize(&XMutex[0], InsPtr[0], XPAR_MUTEX_0_IF_0_BASEADDR);//Mutex0 initialize
	if(xmutex_status != XST_SUCCESS){
		xil_printf("\r\n Mutex-0 Initialize Error ");
	}


	//tStart = XTmrCtr_GetValue(&TimerCounterInst, 0);
	//Smooth();
	//tEnd = XTmrCtr_GetValue(&TimerCounterInst, 0);

	//WriteImageFile();

	//Show_Time(tEnd,tStart);

	for (i=0;i<2;i++) {//フィルタをかける回数

		if(flag==0){
				Once();//BMPファイル読み込み
				endflag = Smooth(XMutex[0]);

				/*while(1){
					xmutex_status = Mutex_GetUser(&XMutex[1],XPAR_MUTEX_0,&User);
					if(User == 0){
						xmutex_status = Mutex_IsLocked(&XMutex[0],XPAR_MUTEX_0);//Mutex0をロック
						xmutex_status = Mutex_SetUser(&XMutex[0],XPAR_MUTEX_0,2);//Mutex0のユーザレジスタを2にして処理終了を知らせる.
						xmutex_status = Mutex_Unlock(&XMutex[0],XPAR_MUTEX_0);//Mutex0のユーザレジスタをUnlock
						flag = 1;
						break;
					}
				}*/
				xmutex_status = Mutex_IsLocked(&XMutex[0],XPAR_MUTEX_0);//Mutex0をロック
				xmutex_status = Mutex_SetUser(&XMutex[0],XPAR_MUTEX_0,2);//Mutex0のユーザレジスタを2にして処理終了を知らせる.
				xmutex_status = Mutex_Unlock(&XMutex[0],XPAR_MUTEX_0);//Mutex0のユーザレジスタをUnlock
				flag = 1;
		}

		while(1){
		xmutex_status = Mutex_GetUser(&XMutex[0],XPAR_MUTEX_1,&User);//Mutex1のユーザレジを監視

			if(User == 1){//9ピクセル分の処理が終わった

				/*dramから値を読む際にMutex1のユーザレジスタを0にする*/
				//xmutex_status = Mutex_IsLocked(&XMutex[1],XPAR_MUTEX_1);
				rd_dram();
				//xmutex_status = Mutex_SetUser(&XMutex[1],XPAR_MUTEX_1,0);
				//xmutex_status = Mutex_Unlock(&XMutex[1],XPAR_MUTEX_1);

				ReadImageFile();//画素情報を読む
				endflag = Smooth(XMutex[1]);//戻り値はcore0の状態

					if(endflag == 1){//1:core1の処理終了
						xmutex_status = Mutex_IsLocked(&XMutex[1],XPAR_MUTEX_0);//Mutex0をロック
						xmutex_status = Mutex_SetUser(&XMutex[1],XPAR_MUTEX_0,2);//Mutex0のユーザレジスタを2にして処理終了を知らせる.
						xmutex_status = Mutex_Unlock(&XMutex[1],XPAR_MUTEX_0);//Mutex0のユーザレジスタをUnlock
					}
			}

			if(User == 2){
				xmutex_status = Mutex_IsLocked(&XMutex[1],XPAR_MUTEX_1);//Mutex1をロック
				xmutex_status = Mutex_SetUser(&XMutex[1],XPAR_MUTEX_1,0);//Mutex1のユーザレジスタを2にして処理終了を知らせる.
				xmutex_status = Mutex_Unlock(&XMutex[1],XPAR_MUTEX_1);//Mutex1のユーザレジスタをUnlock
				break;
			}

		}

	}

	WriteImageHeader();//ヘッダー情報を書きこむ

	xil_printf("\r\n MB i0 End");

	return 0;
}

int Smooth(XMutex InstancePtr)
{
	int i,j,k,tmp=0,endflag=0;
	u32 User;

	unsigned char *outp = (outIMG + 54);

	  /*フィルタサイズ3*3*/
	  for(i=1;i<biWidth-1;i++){
			for(j=1;j<biHeight-1;j++){
			    for(k=0;k<3;k++){//RGB

			    	Mutex_GetUser(&InstancePtr,XPAR_MUTEX_1,&User);//Mutex1のユーザレジを監視
			    	if(User == 2){
			    		endflag = 1;
			    	}

			    	tmp = *(inIMG+(biWidth*(i-1)*3)+3*(j-1)+k)//00
			    		 + *(inIMG+(biWidth*(i-1)*3)+(3*j)+k)//01
			    		 + *(inIMG+(biWidth*(i-1)*3)+3*(j+1)+k)//02
			    		 + *(inIMG+(biWidth*i*3)+3*(j-1)+k)
			    		 + *(inIMG+(biWidth*i*3)+(3*j)+k)
			    		 + *(inIMG+(biWidth*i*3)+3*(j+1)+k)
						 + *(inIMG+(biWidth*(i+1)*3)+3*(j-1)+k)
						 + *(inIMG+(biWidth*(i+1)*3)+(3*j)+k)
						 + *(inIMG+(biWidth*(i+1)*3)+3*(j+1)+k);

			    	*(outp +(biWidth*i*3)+(3*j)+k)= tmp/9;//9画素の平均値を注目画素とする

					//*outIMG = tmp/9;//9画素の平均値を注目画素とする
					//outIMG++;

			    	if(i==1){//最初の行のピクセルをコピーする
			    		*(outp+(biWidth*(i-1))+(j-1)+k) = *(inIMG+(biWidth*(i-1))+(j-1)+k);
			    		//*(wp+(biWidth*(i-1))+(3*(j-1))+k) = *(inIMG+(biWidth*(i-1))+(3*(j-1))+k);
			    	}
			    	if(i==biWidth-2){//最後の行のピクセルをコピーする
			    		*(outp+(biWidth*(i+1)*3)+(j-1)+k) = *(inIMG+(biWidth*(i+1)*3)+(j-1)+k);
			    		//*(wp+(biWidth*(i+1)*3)+(3*(j-1))+k) = *(inIMG+(biWidth*(i+1)*3)+(3*(j-1))+k);
			    	}
			    	if(j==1){//左1列のピクセルをコピーする
			    		*(outp+(biWidth*(i-1)*3)+(j-1)+k) = *(inIMG+(biWidth*(i-1)*3)+(j-1)+k);
			    		//*(wp+(biWidth*(i-1)*3)+(3*(j-1))+k) = *(inIMG+(biWidth*(i-1)*3)+(3*(j-1))+k);
			    	}
			    	if(j==biHeight-2){//右1列のピクセルをコピーする
			    		*(outp+((biWidth*i*3)-1)+(i-1)+k) = *(inIMG+((biWidth*i*3)-1)+(i-1)+k);
			    		//*(wp+(biWidth*i*3)+(3*(i-1))+k) = *(inIMG+(biWidth*i*3)+(3*(i-1))+k);
			    	}

			    	/*縦横9ピクセル分の処理が完了したら*/
			    	if(i==3 && j==3 && k==2){
			    		Mutex_IsLocked(&InstancePtr,XPAR_MUTEX_0);//Mutex0をロック
			    		Mutex_SetUser(&InstancePtr,XPAR_MUTEX_0,1);//Mutex0のユーザレジスタを1にする.
						wr_dram();
			    		Mutex_Unlock(&InstancePtr,XPAR_MUTEX_0);//Mutex0のユーザレジスタをUnlock
			    	}

				}
			}
	  }

	  return endflag;
	  //outIMG = outIMG-((biWidth-2)*(biHeight-2)*3)-54;//先頭アドレスに戻す
	  //dramPtr = (char *)outIMG;
}

void ReadImageHeader()
{
  int i;
  char *cp;

  for(i=0;i<14;i++){//ファイルヘッダーを読み出し(14バイト)
	  BitMapFileHeader[i] = *OriginalPicPtr;
	  OriginalPicPtr++;//アドレスのポインタを更新
  }

  //for(i=0;i<4; i++){//情報ヘッダーにあるサイズを読み込む(4バイト)
	  cp = (char *)&biSize;
	  for(i=0;i<4; i++){
		  *(cp+i) = *OriginalPicPtr;
		  OriginalPicPtr++;
	  }

  //情報ヘッダーにある幅を保存(4バイト)
	  //biWidth = *((int *)dramPtr);
	  cp = (char *)&biWidth;
	  for(i=0;i<4; i++){
		  *(cp+i) = *OriginalPicPtr;
		  OriginalPicPtr++;
	  }

  //for(i=0;i<4; i++){//情報ヘッダーにある高さを保存(4バイト)
	  cp = (char *)&biHeight;
	  for(i=0;i<4; i++){
		  *(cp+i) = *OriginalPicPtr;
		  OriginalPicPtr++;
	  }

  for(i=0;i<28; i++){//残りの情報ヘッダーを保存(28バイト)
	  BitMapInfoHeader[i] = *OriginalPicPtr;
	  OriginalPicPtr++;
  }

  //inIMG = inIMG - (biWidth*biHeight*3);//アドレスのポインタを先頭に戻す(0x80100000)
  OriginalPicPtr = OriginalPicPtr - 54;//アドレスのポインタを先頭に戻す(0x80000000)
}

void Once()
{
	int i,j,k;

	  unsigned char *dramp = (OriginalPicPtr + 54);

	  /*原画像の画素情報を読み込んで3次元配列に保存*/
	  	  for(i = 0; i < biHeight; i++){ //0から幅まで
	  	    for(j = 0; j < biWidth; j++){ //0から高さまで
	  	      for(k = 0; k < 3; k++){ //RGBのそれぞれ
	  	         //inIMG[i][j][k] = *dramPtr;//画素の情報を読み込んで保存する
	  	    	  //*(inIMG+(biWidth*(i+k) + j)) = *dramPtr;//画素の情報を読み込んで保存する
	  	    	  *inIMG = *dramp;
	  	    	  inIMG++;
	  	    	  dramp++;
	  	      }
	  	    }
	  	  }
	  	inIMG = inIMG - (biWidth*biHeight*3);
}

void ReadImageFile()
{
  int i,j,k;

  unsigned char *dramp = (dramPtr + 54);

  /*前の画像の画素情報を読み込んでアドレスに保存*/
	  for(i = 0; i < biHeight; i++){ //0から幅まで
	    for(j = 0; j < biWidth; j++){ //0から高さまで
	      for(k = 0; k < 3; k++){ //RGBのそれぞれ
	    	  *inIMG = *dramp;
	    	  inIMG++;
	    	  dramp++;
	      }
	    }
	  }
  inIMG = inIMG - (biWidth*biHeight*3);//アドレスのポインタを先頭に戻す
  //dramPtr = dramPtr - (biWidth*biHeight*3) - 54;//アドレスのポインタを先頭に戻す
}

void WriteImageHeader()
{
  int i;
  char *cp;


  for(i=0;i<14;i++){//ファイルヘッダーを書き込む
	  //*dramPtr = BitMapFileHeader[i];
  	  //dramPtr++;//アドレスのポインタを更新
	  *outIMG = BitMapFileHeader[i];
	  outIMG++;
  }

  //情報ヘッダーにあるサイズを書きこむ(4バイト)
	  //*dramPtr = biSize;
	  //dramPtr = dramPtr+4;

	  cp = (char *)&biSize;
	  for(i=0;i<4; i++){
		  *outIMG = *(cp+i);
		  outIMG++;
	  }

  //for(i=0;i<4; i++){//情報ヘッダーに幅を書きこむ(4バイト)
	  //*dramPtr = biWidth;
	  //dramPtr= dramPtr+4;

	  cp = (char *)&biWidth;
	  for(i=0;i<4; i++){
		  *outIMG = *(cp+i);
		  outIMG++;
	  }
  //}

  //for(i=0;i<4; i++){//情報ヘッダーに高さを書きこむ(4バイト)
	  //*dramPtr = biHeight;
	  //dramPtr = dramPtr+4;

	  cp = (char *)&biHeight;
	  for(i=0;i<4; i++){
		  *outIMG = *(cp+i);
		  outIMG++;
	  }
  //}
  for(i=0;i<28; i++){//残りの情報ヘッダーを書きこむ(28バイト)
	  //*dramPtr = BitMapInfoHeader[i];
	  //dramPtr++;
	  *outIMG = BitMapInfoHeader[i];
	  outIMG++;
  }
  outIMG-=54;//先頭アドレスに戻す
}

int init()
{
	int Status;

	Status = XTmrCtr_Initialize(&TimerCounterInst, XPAR_AXI_TIMER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Timer Initialize Error \r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetOptions(&TimerCounterInst, 0, XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_Start(&TimerCounterInst, 0);
	return 0;
}

void Show_Time(u32 tEnd, u32 tStart)
{
	double tt = 0.0;
	int m,b;
	//float tt=0.0;

	tt = (1.0 * (double) (tEnd - tStart) / 100000000.0);
	//tt = (1.0 * (float)(tEnd - tStart) / 100000000.0);

	m = tt;//小数点以下を切り捨ててsecとする
	b = (tt - m) * 1000000000;//nsecまで表示

	xil_printf("\r\n(MicroBlaze)");
	xil_printf("\r\n Time=%4d.%09d[s]", m, b);

}

void wr_dram()
{
	/*widthとheightの書きこみ*/
	*dram = biWidth;
	dram++;//4バイトシフト
	*dram = biHeight;
}

void rd_dram()
{
	/*widhtとheightの読み出し*/
	biWidth = *dram;
	dram++;//4バイトシフト
	biHeight = *dram;
}

void Mutex_Lock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 LockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | LOCKED_BIT);
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MutexNumber < InstancePtr->Config.NumMutex);

	/*original*/

	while (1){
		XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET, LockPattern);
		Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress,
					MutexNumber,
					XMU_MUTEX_REG_OFFSET);
		if (Value == LockPattern) {
			break;
		}
	}
}

int Mutex_Trylock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 LockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | LOCKED_BIT);
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_MUTEX_REG_OFFSET, LockPattern);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);
	if (Value == LockPattern) {
		return XST_SUCCESS;
	} else{
		return XST_DEVICE_BUSY;
	}
}

int Mutex_Unlock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 UnLockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | 0x0);
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);


	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);

	/* Verify that the caller actually owns the Mutex */
	if ((Value & OWNER_MASK) != UnLockPattern) {
		return XST_FAILURE;
	}

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_MUTEX_REG_OFFSET, UnLockPattern);

	return XST_SUCCESS;
}

int Mutex_IsLocked(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);

	return ((int)(Value & LOCKED_BIT));
}

void Mutex_GetStatus(XMutex *InstancePtr, u8 MutexNumber, u32 *Locked,
			u32 *Owner)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MutexNumber < InstancePtr->Config.NumMutex);
	Xil_AssertVoid(Locked != NULL);
	Xil_AssertVoid(Owner != NULL);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
			       XMU_MUTEX_REG_OFFSET);
	*Locked = (Value & LOCKED_BIT);
	*Owner = (Value & OWNER_MASK) >> OWNER_SHIFT;
}

int Mutex_GetUser(XMutex *InstancePtr, u8 MutexNumber, u32 *User)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);
	Xil_AssertNonvoid(User != NULL);

	if (!(InstancePtr->Config.UserReg)) {
		return XST_NO_FEATURE;
	}

	*User = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_USER_REG_OFFSET);

	//xil_printf("\r\n %d ",(*User)*10);

	return XST_SUCCESS;
}

int Mutex_SetUser(XMutex *InstancePtr, u8 MutexNumber, u32 User)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	if (!(InstancePtr->Config.UserReg)) {
		return XST_NO_FEATURE;
	}

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_USER_REG_OFFSET, User);

	/*書いた内容を確認する.*/
	//xil_printf("\r\n %d \n",XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,XMU_USER_REG_OFFSET));

	return XST_SUCCESS;
}
