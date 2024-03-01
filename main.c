#include <xc.h>

//configurations
#pragma config JTAGEN =     OFF
#pragma config FWDTEN =     OFF
#pragma config FNOSC =      FRCPLL
#pragma config FSOSCEN =    OFF
#pragma config POSCMOD =    EC
#pragma config OSCIOFNC =   ON
#pragma config FPBDIV =     DIV_1
#pragma config FPLLIDIV =   DIV_2
#pragma config FPLLMUL =    MUL_20
#pragma config FPLLODIV =   DIV_1

#define BTNU                    PORTBbits.RB1
#define BTNC                    PORTFbits.RF0
#define BTND                    PORTAbits.RA15
#define LCD_FIRST_LINE_START    0x80
#define LCD_FIRST_LINE_END      0x8F
#define LCD_SECOND_LINE_START   0xc0
#define LCD_SECOND_LINE_END     0xcF
#define LEFT                    1
#define RIGHT                   0

#define ENABLE() {\
    PORTDbits.RD4 = 1;\
    PORTDbits.RD4 = 0;\
    busy();}

#define CONTROL() {\
    PORTBbits.RB15 = 0; /* RS = 0 */\
    PORTDbits.RD5 = 0;} /* RW = 0 */

#define DATA() {\
    PORTBbits.RB15 = 1; /* RS = 1 */\
    PORTDbits.RD5 = 0;} /* RW = 0 */


void initSpeaker();
void initButtons();
void initLCD();
void delay();
void writeLCD(char string[], int size);
void clearLCD();
void moveCursor(int pos);
void beep();
void configureCG(char string[], int size, int pos);
void move(char string[], int pos, int size);
void handleMovement(int* pos, int* direction, char originalString[], int size);
void busy();


void main(){
    initButtons();
    initSpeaker();
    initLCD();
    
    char CG_ALPHABET[64] = {0x00, 0x00, 0x0A, 0x00, 0x11, 0x11, 0x0E, 0x00, //'smiley face'
                            0x20, 0x2A, 0x35, 0x31, 0x31, 0x2A, 0x24, 0x20, // 'heart'
                            0x40, 0x51, 0x51, 0x4A, 0x4C, 0x52, 0x51, 0x40,  // 'א'
                            0x60, 0x7F, 0x71, 0x71, 0x7D, 0x61, 0x7F, 0x60,  // 'פ'
                            0x80, 0x95, 0x95, 0x95, 0x95, 0x95, 0x9F, 0x80,  // 'ש'
                            0xA0, 0xB2, 0xB5, 0xB5, 0xB1, 0xB1, 0xAE, 0xA0,  // 'ט'
                            0xC0, 0xC7, 0xC1, 0xC1, 0xC0, 0xC0, 0xC0, 0xC0,  // 'י'
                            0xE0, 0xE7, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1,  // 'ן'
    };
    
    configureCG(CG_ALPHABET, 64, LCD_FIRST_LINE_START);
    
    int pos = LCD_FIRST_LINE_START;
    int direction = 0;
    int current = 0;
    int size;
    
    char menachem[9][9] = {{0},{1},{2},{3},{4},{5},{6},{7},{1,7,6,6,5,4,3,2,1}};

    while(1){
        
        // Change char on LCD
        if(BTNU){
            clearLCD();
            moveCursor(pos);
            
            current++;
            // return to array start
            if( current == 9) current = 0;
          
                
        }
        
        // Switch line on screen
        if(BTND){
            if(pos < LCD_SECOND_LINE_START){
                pos = LCD_SECOND_LINE_START;
            } else {
                pos = LCD_FIRST_LINE_START;
            }
            moveCursor(pos);
        }
        
        if(BTNC){
            beep();
        }
        if(current == 8){
            size = 9;
        } else {
            size = 1;
        }
        handleMovement(&pos, &direction, menachem[current], size);
        delay();
    }
    
}



void move(char string[], int pos, int size){
    clearLCD();
    moveCursor(pos);
    writeLCD(string, size);
}

void handleMovement(int* pos, int* direction, char originalString[], int size) {
    char* message = originalString;
    move(message, *pos, size);
    
    if(*pos == LCD_SECOND_LINE_END || *pos == LCD_FIRST_LINE_END)
        *direction = 1; //Move left
      
    if(*pos == LCD_SECOND_LINE_START || *pos == LCD_FIRST_LINE_START)
        *direction = 0; //Move right
       
    if (*direction == 0)
        (*pos)++;
    else
        (*pos)--;
}

void busy(void){
    // Backup
    char RD,RS;
    int STATUS_TRISE;
    int portMap;
    RD = PORTDbits.RD5;
    RS = PORTBbits.RB15;
    STATUS_TRISE = TRISE;
    
    PORTDbits.RD5 = 1; // Read
    PORTBbits.RB15 = 0; // Control
    portMap = TRISE;
    portMap |= 0x80;
    TRISE = portMap;
    do{
        PORTDbits.RD4 = 1; // Enable=1
        PORTDbits.RD4 = 0; // Enable=0
    } while(PORTEbits.RE7); // BF
    
    // Restore from backup
    PORTDbits.RD5 = RD;
    PORTBbits.RB15 = RS;
    TRISE = STATUS_TRISE;
}

void initButtons() {
    // Button configuration
    TRISBbits.TRISB1 = 1; // BTNU configured as input
    TRISFbits.TRISF0 = 1; // BTNC configured as input
    TRISAbits.TRISA15 = 1; // BTND configured as input
    ANSELBbits.ANSB1 = 0; // Disabel BTNU analog
}


void initSpeaker(){
    // Speaker Buttons
    TRISBbits.TRISB14 = 0; //Speaker configured as output
    ANSELBbits.ANSB14 = 0; //Disable speaker analog
}

void initLCD(){
    
    TRISBbits.TRISB15 = 0; // RB15 (DISP_RS) set as an output
    ANSELBbits.ANSB15 = 0; // disable analog functionality on RB15 (DISP_RS)
    TRISDbits.TRISD5 = 0; // RD5 (DISP_RW) set as an output
    TRISDbits.TRISD4 = 0; // RD4 (DISP_EN) set as an output
    ANSELEbits.ANSE2 = 0; // Disable corresponding analog
    ANSELEbits.ANSE4 = 0; // Disable corresponding analog
    ANSELEbits.ANSE5 = 0; // Disable corresponding analog
    ANSELEbits.ANSE6 = 0; // Disable corresponding analog
    ANSELEbits.ANSE7 = 0; // Disable corresponding analog
    TRISE&=0xff00;
    
    CONTROL();
    char control[] = {0x38,0x38,0x38,0xe,0x6,0x1,0x40};
    for(int i = 0 ; i < sizeof(control) ; i++) {
        PORTE = control[i]; // Set a certain control operation
        ENABLE();
     }
}

void writeLCD(char string[], int size){
    DATA();
    for(int i = 0; i < size; i++) {
        PORTE = string[i];  // Buffer
        ENABLE();
    }
}

void moveCursor(int pos){
    CONTROL();
    PORTE = pos;
    ENABLE();
}

void clearLCD(){
    CONTROL();
    PORTE = 0x01;
    ENABLE();
}

void delay(){
    for(int i = 0; i < 262000; i++);
}

void beep(){
    int j=100,i=100;
    while(1){
      PORTBbits.RB14 ^= 1;
    
        for(;j<100+i;j++);
            i+=100;

        if(i>320000)
        {
            i=100;
            j=10;
            return;
        }
    }
}

void configureCG(char string[], int size, int pos){
    DATA();
    for(int i = 0; i < size; i++){
        PORTE = string[i];
        ENABLE();
    }
    
    CONTROL();
    PORTE = pos;
    ENABLE();
}
