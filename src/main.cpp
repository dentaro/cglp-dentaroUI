#include <LovyanGFX_DentaroUI.hpp>
// #include <Key.h>
// #include <Keypad.h>

// const byte ROWS = 3; //four rows
// const byte COLS = 4; //four columns
// char hexaKeys[ROWS][COLS] = {
//   {'0','3','4','a'},//←  ,↓  ,on , 
//   {'1','2','b','6'},//→  ,↑  ,   ,reset
//   {'7','9','8','5'} //snd,   ,o  ,sp
// };
// byte rowPins[ROWS] = {22,33,3}; //connect to the row pinouts of the keypad
// byte colPins[COLS] = {26,0,5,27}; //connect to the column pinouts of the keypad
// //34~39までのPINはインプットのみなのでつかえない
// //initialize an instance of class NewKeypad
// Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

int pressedBtnID = -1;//この値をタッチボタン、物理ボタンの両方から操作してbtnStateを間接的に操作している

//上から34,36,39
// #define BUTTON_A_PIN 39
// #define BUTTON_B_PIN 36


#include <float.h>
#include "cglp.h"
#include "machineDependent.h"

static LGFX lcd;
LovyanGFX_DentaroUI ui(&lcd);
static LGFX_Sprite sideSprite( &lcd );//背景スプライトはディスプレイに出力
static LGFX_Sprite canvas(&lcd);
static bool isCanvasCreated = false;
static int canvasX;
static int canvasY;

typedef struct {
  float freq;
  float duration;
  float when;
} SoundTone;

#define TONE_PER_NOTE 32
#define SOUND_TONE_COUNT 64
static SoundTone soundTones[SOUND_TONE_COUNT];
static int soundToneIndex = 0;
static float soundTime = 0;

static void initSoundTones() {
  for (int i = 0; i < SOUND_TONE_COUNT; i++) {
    soundTones[i].when = FLT_MAX;
  }
}

static void addSoundTone(float freq, float duration, float when) {
  SoundTone *st = &soundTones[soundToneIndex];
  st->freq = freq;
  st->duration = duration;
  st->when = when;
  soundToneIndex++;
  if (soundToneIndex >= SOUND_TONE_COUNT) {
    soundToneIndex = 0;
  }
}

void md_drawRect(float x, float y, float w, float h, unsigned char r,
                 unsigned char g, unsigned char b) {
  canvas.fillRect((int)x, (int)y, (int)w, (int)h, lcd.color565(r, g, b));
}

#define TRANSPARENT_COLOR 0

typedef struct {
  LGFX_Sprite *sprite;
  int hash;
} CharaterSprite;

static CharaterSprite characterSprites[MAX_CACHED_CHARACTER_PATTERN_COUNT];
static int characterSpritesCount;

static void initCharacterSprite() {
  for (int i = 0; i < MAX_CACHED_CHARACTER_PATTERN_COUNT; i++) {
    characterSprites[i].sprite = new LGFX_Sprite(&canvas);
  }
  characterSpritesCount = 0;
}

static void resetCharacterSprite() {
  for (int i = 0; i < characterSpritesCount; i++) {
    characterSprites[i].sprite->deleteSprite();
  }
  characterSpritesCount = 0;
}

static uint16_t characterImageData[CHARACTER_WIDTH * CHARACTER_HEIGHT];

static void createCharacterImageData(
    unsigned char grid[CHARACTER_HEIGHT][CHARACTER_WIDTH][3]) {
  int cp = 0;
  for (int y = 0; y < CHARACTER_HEIGHT; y++) {
    for (int x = 0; x < CHARACTER_WIDTH; x++) {
      unsigned char r = grid[y][x][0];
      unsigned char g = grid[y][x][1];
      unsigned char b = grid[y][x][2];
      characterImageData[cp] =
          (r > 0 || g > 0 || b > 0) ? lcd.color565(r, g, b) : TRANSPARENT_COLOR;
      cp++;
    }
  }
}

void md_drawCharacter(unsigned char grid[CHARACTER_HEIGHT][CHARACTER_WIDTH][3],
                      float x, float y, int hash) {
  CharaterSprite *cp = NULL;
  for (int i = 0; i < characterSpritesCount; i++) {
    if (characterSprites[i].hash == hash) {
      cp = &characterSprites[i];
      break;
    }
  }
  if (cp == NULL) {
    cp = &characterSprites[characterSpritesCount];
    cp->hash = hash;
    createCharacterImageData(grid);
    cp->sprite->createSprite(CHARACTER_WIDTH, CHARACTER_HEIGHT);
    cp->sprite->setSwapBytes(true);
    cp->sprite->pushImage(0, 0, CHARACTER_WIDTH, CHARACTER_HEIGHT,
                          characterImageData);
    characterSpritesCount++;
  }
  cp->sprite->pushSprite((int)x, (int)y, TRANSPARENT_COLOR);
}

void md_clearView(unsigned char r, unsigned char g, unsigned char b) {
  canvas.fillSprite(lcd.color565(r, g, b));
}

void md_clearScreen(unsigned char r, unsigned char g, unsigned char b) {
  lcd.fillRect(0,0,240,240,lcd.color565(r, g, b));
  // lcd.fillScreen(lcd.color565(r, g, b));
}

void md_playTone(float freq, float duration, float when) {
  addSoundTone(freq, duration, when);
}

void md_stopTone() {
  initSoundTones();
  // M5.Beep.mute();
}

float md_getAudioTime() { return soundTime; }

void md_consoleLog(char *msg) { Serial.println(msg); }

void md_initView(int w, int h) {
  if (isCanvasCreated) {
    canvas.deleteSprite();
  }
  isCanvasCreated = true;
  canvas.createSprite(w, h);
  // if (w > 135) {
  //   lcd.setRotation(1);
  // } else {
  //   lcd.setRotation(0);
  // }
  canvasX = (240 - w) / 2 ;
  canvasY = (lcd.height() - h) / 2;
  resetCharacterSprite();
}


uint8_t getPhBtnInput(){
  // uint8_t buttons = 0;
 pressedBtnID = -1;

if(ui.getPhVolVec(0, 1)==7){pressedBtnID = 0;}
if(ui.getPhVolVec(0, 1)==3){pressedBtnID = 1;}
if(ui.getPhVolVec(0, 1)==5){pressedBtnID = 2;}
if(ui.getPhVolVec(0, 1)==1){pressedBtnID = 3;}

if(ui.getPhVolDir(2)==0){pressedBtnID = 4;}
if(ui.getPhVolDir(2)==3){pressedBtnID = 5;}
if(ui.getPhVolDir(2)==5){pressedBtnID = 6;}
if(ui.getPhVolDir(2)==1){pressedBtnID = 7;}


ui.updatePhBtns();//物理ボタン3つ

if(ui.getHitValue() == 101||ui.getHitValue() == 111){pressedBtnID = 8;}
if(ui.getHitValue() == 102||ui.getHitValue() == 112){pressedBtnID = 9;}
if(ui.getHitValue() == 103||ui.getHitValue() == 113){pressedBtnID = 10;}

  return pressedBtnID;
}


static TaskHandle_t frameTaskHandle;

uint32_t preTime = 0;

uint64_t frame = 0;
//2倍拡大表示用のパラメータ
float matrix_game[6] = {2.0,   // 横2倍
                     -0.0,  // 横傾き
                     0.0,   // X座標
                     0.0,   // 縦傾き
                     2.0,   // 縦2倍
                     0.0    // Y座標
                    };

static void updateFromFrameTask() {

  ui.update(lcd);//タッチイベントを取るので、LGFXが基底クラスでないといけない

  // if( ui.getEvent() != NO_EVENT ){//何かイベントがあれば
  //   if( ui.getEvent() == TOUCH ){//TOUCHの時だけ
  //   }
  //   if(ui.getEvent() == MOVE){
  //     pressedBtnID = ui.getTouchBtnID()+12;//12個分の物理ボタンをタッチボタンIDに足す
  //   }
  //   if( ui.getEvent() == RELEASE ){//RELEASEの時だけ
  //     // pressedBtnID = ui.getTouchBtnID()+12;//12個分の物理ボタンをタッチボタンIDに足す
  //     pressedBtnID = -1;//リセット
  //   }
  // }

  // ui.updatePhBtns();//物理ボタンの状態を更新

  // if(customKeypad.getState() == RELEASED){
  //   pressedBtnID = -1;//リセット
  // }

  // char customKey  = '_';
  //   customKey = customKeypad.getKey();
    
  //   if (customKey){
        
  //       if(customKey == '0'){
  //           pressedBtnID = 0;
  //       }else if(customKey == '1'){
  //           pressedBtnID = 1;
  //       }else if(customKey == '2'){
  //           pressedBtnID = 2;
  //       }else if(customKey == '3'){
  //           pressedBtnID = 3;
  //       }else if(customKey == '4'){
  //           pressedBtnID = 4;
  //       }else if(customKey == '5'){
  //           pressedBtnID = 5;
  //       }else if(customKey == '6'){
  //           pressedBtnID = 6;
  //       }else if(customKey == '7'){
  //           pressedBtnID = 7;
  //       }else if(customKey == '8'){
  //           pressedBtnID = 8;
  //       }
  //   }
    // canvas.print(customKey);

  ui.updatePhVols();
  ui.updatePhBtns();//物理ボタンの状態を更新
  // uint32_t hitvalue = ui.getHitValue();
  // // 入力内容を画面とシリアルに出力
    // switch (hitvalue)
    // {
    // default:  Serial.println("--"); break;
    // case 101: Serial.println("A click"); break;
    // case 102: Serial.println("B click"); break;
    // case 103: Serial.println("C click"); break;
    // case 111: Serial.println("A hold"); break;
    // case 112: Serial.println("B hold"); break;
    // case 113: Serial.println("C hold"); break;
    // case 121: Serial.println("A float click"); break;
    // case 122: Serial.println("B float click"); break;
    // case 123: Serial.println("C float click"); break;
    // case 201: Serial.println("AB hold"); break;
    // case 202: Serial.println("AC hold"); break;
    // case 203: Serial.println("BC hold"); break;
    // case 204: Serial.println("ABC hold"); break;
    // case 301: Serial.println("A->B->C"); break;
    // }

  // ui.setConstantGetF(true);//trueだとタッチポイントのボタンIDを連続取得するモード
  // ui.update(lcd);//タッチイベントを取るので、LGFXが基底クラスでないといけない

  // if( ui.getEvent() != NO_EVENT ){//何かイベントがあれば

  //   if( ui.getEvent() == TOUCH ){//TOUCHの時だけ
      
  //   }
  //   if(ui.getEvent() == MOVE){
  //     //ui.getTouchBtnID();
  //   }
  //   if( ui.getEvent() == RELEASE ){//RELEASEの時だけ
  //     // ui.setBtnID(-1);//タッチボタンIDをリセット
  //     // pressedBtnID = ui.getTouchBtnID()+12;//12個分の物理ボタンをタッチボタンIDに足す
  //     pressedBtnID = -1;//リセット
  //   }
  // }

  pressedBtnID = getPhBtnInput();

    // if(ui.getTouchBtnID() == RELEASE){//リリースされたら
    //   pressedBtnID = -1;
    // }
  
  uint32_t now = millis();
  uint32_t remainTime= (now - preTime);
  preTime = now;

  //ゲーム内のprint時の文字設定をしておく
  canvas.setTextSize(1);//サイズ
  canvas.setFont(&lgfxJapanGothicP_8);//日本語可
  canvas.setCursor(0, 0);//位置
  canvas.setTextWrap(true);

  //0ボタンで強制終了
  if (pressedBtnID == 10)
  { // reload

  }

  // if (pressedBtnID == 9999)
  // { // reload

  // }


        ui.showTouchEventInfo( canvas, 0, 100 );//タッチイベントを視覚化する
        ui.showInfo( canvas, 0, 100+8 );//ボタン情報、フレームレート情報などを表示します。
        // ui.drawPhBtns( canvas, 0, 90+16 );//物理ボタンの状態を表示

        //SD利用可能かどうか
        // canvas.setTextSize(1);
        // canvas.setTextColor( TFT_WHITE , TFT_BLACK );
        // canvas.setCursor( 100, 0 );
        // if(isCardMounted){
        //   canvas.print("SDアリ");
        // }else{
        //   canvas.print("SDナシ");
        // }

          

        
        // delay(4);//120FPS スプライトが少ない、速度の速いモード描画ぶん待つ
        // delay(10);//30FPS メニュー、パズルなど

  // int wait = 1000/60 - remainTime;//フレームレートを60FPS合わせる
  // if(wait > 0){
  //   delay(wait);
  // }
  // xSemaphoreGiveFromISR(semaphore, NULL);

  bool bu = false;
  bool bd = false;
  bool br = false;
  bool bl = false;
  bool ba = false;
  bool bb = false;

  if(pressedBtnID == -1){
    bool bu = false;
    bool bd = false;
    bool br = false;
    bool bl = false;
    bool ba = false;
    bool bb = false;
  }

  // ba = !lgfx::gpio_in(BUTTON_A_PIN);
  // bb = !lgfx::gpio_in(BUTTON_B_PIN);
  
  if(pressedBtnID == 0){
    bl= true;
  }
  if(pressedBtnID == 1){
    br = true;
  }
  if(pressedBtnID == 2){
    bu = true;
  }
  if(pressedBtnID == 3){
    bd = true;
  }

  if(pressedBtnID ==6 || pressedBtnID == 5){
    ba = true;
  }
  if(pressedBtnID == 4|| pressedBtnID == 7){
    bb = true;
  }

  setButtonState(bl, br, bu, bd, bb, ba);
  updateFrame();
  lcd.startWrite();
  // canvas.pushSprite(canvasX, canvasY);
  canvas.pushAffine(matrix_game);//ゲーム画面を最終描画する
  //Affineを使わない書き方
  // canvas.setPivot(0, 0);
  // canvas.pushRotateZoom(&screen, 0, 0, 0, 2, 2);
  // canvas.pushSprite(&lcd,canvasX,canvasY);//ゲーム画面を小さく高速描画する

  lcd.endWrite();
  if (!isInMenu) {
    if(pressedBtnID == 10){
      goToMenu();
    }
    if(pressedBtnID == 9){
      toggleSound();
    }
  }
  // if (!isInMenu) {
  //   if (currentInput.b.isJustPressed) {
  //     if (currentInput.a.isPressed) {
  //       goToMenu();
  //     } else {
  //       toggleSound();
  //     }
  //   }
  // }
  frame++;

  if(frame > 18446744073709551615)frame = 0;

  delay(1);

}

static void updateFrameTask(void *pvParameters) {
  while (1) {
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    updateFromFrameTask();
  }
}

static void IRAM_ATTR onFrameTimer() {
  xTaskNotifyFromISR(frameTaskHandle, 0, eIncrement, NULL);
}

static TaskHandle_t soundTaskHandle;

static void updateFromSoundTask() {
  // M5.Beep.update();
  soundTime += 60 / tempo / TONE_PER_NOTE;
  float lastWhen = 0;
  int ti = -1;
  for (int i = 0; i < SOUND_TONE_COUNT; i++) {
    SoundTone *st = &soundTones[i];
    if (st->when <= soundTime) {
      if (st->when > lastWhen) {
        ti = i;
        lastWhen = st->when;
        st->when = FLT_MAX;
      }
    }
  }
  if (ti >= 0) {
    SoundTone *st = &soundTones[ti];
    // M5.Beep.tone((uint16_t)st->freq, (uint32_t)(st->duration * 1000));
  }
}

static void updateSoundTask(void *pvParameters) {
  while (1) {
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    updateFromSoundTask();
  }
}

// boolean blink = false;
// // Taking care of some special events.
// void keypadEvent(KeypadEvent key){
//     switch (customKeypad.getState()){
//     case PRESSED:
//     Serial.println("pressed");
//         break;

//     case RELEASED:
//     Serial.println("released");
//         break;

//     case HOLD:
//     Serial.println("hold");
//         break;
//     }
// }

static void IRAM_ATTR onSoundTimer() {
  xTaskNotifyFromISR(soundTaskHandle, 0, eIncrement, NULL);
}

void setup() {
  Serial.begin(115200);
  // M5.begin();
  // ui.begin( lcd, 16, 1, false);
  ui.begin( lcd, 16, 1, false);
  
  delay(100);

  // customKeypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  initCharacterSprite();
  initSoundTones();
  disableSound();
  initGame();
  hw_timer_t *frameTimer = NULL;
  frameTimer = timerBegin(0, getApbFrequency() / FPS / 1000, true);
  timerAttachInterrupt(frameTimer, &onFrameTimer, true);
  timerAlarmWrite(frameTimer, 1000, true);
  timerAlarmEnable(frameTimer);
  xTaskCreateUniversal(updateFrameTask, "updateFrameTask", 8192, NULL, 1,
                       &frameTaskHandle, APP_CPU_NUM);
  hw_timer_t *soundTimer = NULL;
  soundTimer = timerBegin(
      1, getApbFrequency() * (60.0f / tempo / TONE_PER_NOTE) / 1000, true);
  timerAttachInterrupt(soundTimer, &onSoundTimer, true);
  timerAlarmWrite(soundTimer, 1000, true);
  timerAlarmEnable(soundTimer);
  xTaskCreateUniversal(updateSoundTask, "updateSoundTask", 8192, NULL, 2,
                       &soundTaskHandle, PRO_CPU_NUM);

  //ボタン配置(見えない)
  // ui.createBtns( 240,  0,  60, 240,  1, 4, TOUCH );//コントローラー4ボタン

  sideSprite.setPsram(false );
  sideSprite.setColorDepth(16);//子スプライトの色深度
  sideSprite.createSprite(80, 240);//ゲーム画面用スプライトメモリ確保
  sideSprite.fillScreen(TFT_LIGHTGRAY);
  for(int j = 0; j < 4; j++)sideSprite.drawRoundRect(0, 60*j, 80, 60, 4, TFT_BLACK);
  sideSprite.pushSprite( &lcd, 240, 0 );
  sideSprite.deleteSprite();//描画したらメモリを解放する
}

void loop() { 
  
    // char key = customKeypad.getKey();
    // if (key) {
    //   Serial.print(blink);
    //   Serial.println(key);
    // }


  // ui.update(lcd);//タッチイベントを取るので、LGFXが基底クラスでないといけない

  // if( ui.getEvent() != NO_EVENT ){//何かイベントがあれば
  //   if( ui.getEvent() == TOUCH ){//TOUCHの時だけ
  //   }
  //   if(ui.getEvent() == MOVE){
  //     pressedBtnID = ui.getTouchBtnID()+12;//12個分の物理ボタンをタッチボタンIDに足す
  //   }
  //   if( ui.getEvent() == RELEASE ){//RELEASEの時だけ
  //     pressedBtnID = ui.getTouchBtnID()+12;//12個分の物理ボタンをタッチボタンIDに足す
  //   }
  // }
  
  delay(1000); 
  }