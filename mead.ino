/** 
 * 2015/04/21
 *
 * HTE 能量檢測儀器,USB to TTL 晶片為 PL2303
 */

/* USB 連線相關 */
#include <usbhub.h>
#include <cdcacm.h>
#include <cdcprolific.h>

// Soft Serial Port
#include <SoftwareSerial.h>

// PL 晶片 Class
class PLAsyncOper :
public CDCAsyncOper
{
  public:
    virtual uint8_t OnInit(ACM *pacm);
};

uint8_t PLAsyncOper::OnInit(ACM *pacm)
{
  uint8_t rcode;

  // Set DTR = 1
  rcode = pacm->SetControlLineState(1);

  if (rcode)
  {
    ErrorMessage<uint8_t>(PSTR("SetControlLineState"), rcode);
    return rcode;
  }

  LINE_CODING lc;
  lc.dwDTERate = 9600;	
  lc.bCharFormat = 0;
  lc.bParityType = 0;
  lc.bDataBits	= 8;	

  rcode = pacm->SetLineCoding(&lc);

  if (rcode)
    ErrorMessage<uint8_t>(PSTR("SetLineCoding"), rcode);

  return rcode;
}

// 參數設定
USB Usb;
PLAsyncOper  AsyncOper;
PL2303       Pl(&Usb, &AsyncOper); // USB裝置
SoftwareSerial serialBT(2, 3);  // RX, TX, 藍芽

uint8_t intVal = 0;  // buffer 區的 value

void setup()
{
  // 初始相關 serial port
  //Serial.begin(9600);
  serialBT.begin(9600);

  // USB初始，檢查是否裝置錯誤  
  Usb.Init();
  
  /*
  if (Usb.Init() == -1) 
    // Serial.println("err_usb");
  else
   // Serial.println("device ready");
  */

  delay(500); 
}

void loop()
{
  runUSB();

  //delay(100);
}  

/* 接收USB 傳送的資料,主要為 int 的資料 */
void runUSB()
{
  Usb.Task();

  if( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {  
    uint8_t rcode;
    uint8_t buf[8];           
    uint16_t rcvd = 8;
    rcode = Pl.RcvData(&rcvd, buf);   

    if (rcode && rcode != hrNAK)
      ErrorMessage<uint8_t>(PSTR("Ret"), rcode);

    // more than zero bytes received
    if( rcvd ) { 
      for(uint16_t i=0; i < rcvd; i++ ) {
        if ( (int)buf[i] != 0) {
          intVal = (int)buf[i];
          serialBT.write(intVal); 

          delay(5);
        }
      }          
    }            
  }  
}
