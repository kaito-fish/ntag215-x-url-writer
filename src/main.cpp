#include <Arduino.h>
#include <Wire.h>
#include <MFRC522_I2C.h>
#include <M5Unified.h>

MFRC522_I2C rfid(0x28, 0, &Wire);

// ----MFRC522_I2C用----
static bool writePage(uint8_t page, const uint8_t data4[4]) {
  MFRC522_I2C::StatusCode st =
      (MFRC522_I2C::StatusCode)rfid.MIFARE_Ultralight_Write(page, (byte*)data4, 4);
  return st == MFRC522_I2C::STATUS_OK;
}
static bool read16(uint8_t startPage, uint8_t out18[18]) {
  byte len = 18;
  MFRC522_I2C::StatusCode st =
      (MFRC522_I2C::StatusCode)rfid.MIFARE_Read(startPage, out18, &len);
  return st == MFRC522_I2C::STATUS_OK;
}

// URL→NDEF URI(TLV付き)
static size_t buildNdefUrl(const String& url, uint8_t* out, size_t maxLen) {
  uint8_t prefix = 0x00;
  String rest = url;

  if (url.startsWith("https://")) { prefix = 0x04; rest = url.substring(8); }
  else if (url.startsWith("http://")) { prefix = 0x03; rest = url.substring(7); }
  else if (url.startsWith("https://www.")) { prefix = 0x02; rest = url.substring(12); }
  else if (url.startsWith("http://www.")) { prefix = 0x01; rest = url.substring(11); }

  const size_t payloadLen = 1 + rest.length();
  const size_t recordLen  = 4 + payloadLen;
  const size_t totalLen   = 2 + recordLen + 1;
  if (totalLen > maxLen) return 0;

  size_t i = 0;
  out[i++] = 0x03;
  out[i++] = (uint8_t)recordLen;
  out[i++] = 0xD1;
  out[i++] = 0x01;
  out[i++] = (uint8_t)payloadLen;
  out[i++] = 0x55;
  out[i++] = prefix;
  for (size_t k = 0; k < rest.length(); k++) out[i++] = (uint8_t)rest[k];
  out[i++] = 0xFE;
  return i;
}

// 画面表示ヘルパ
static void showStatus(const char* line1, const char* line2 = nullptr) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.setTextSize(2);
  M5.Display.println(line1);
  M5.Display.setTextSize(1);
  if (line2) M5.Display.println(line2);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // Grove I2C: SDA=32, SCL=33
  Wire.begin(32, 33);

  rfid.PCD_Init();

  Serial.begin(115200);
  showStatus("Ready", "Tap NTAG215");
}

void loop() {
  M5.update();

  // 待機表示（ずっと点滅させたいならここでUI更新も可）
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // 検出
  char uidstr[64] = {0};
  int pos = 0;
  for (byte i = 0; i < rfid.uid.size; i++) {
    pos += snprintf(uidstr + pos, sizeof(uidstr) - pos, "%02X", rfid.uid.uidByte[i]);
    if (i + 1 < rfid.uid.size) pos += snprintf(uidstr + pos, sizeof(uidstr) - pos, " ");
  }
  showStatus("Tag Found", uidstr);
  delay(400);

  bool okAll = true;

  do {
    showStatus("Reading...");
    uint8_t buf[18];
    if (!read16(0x00, buf)) { okAll = false; break; }

    // CCチェック
    showStatus("Checking CC...");
    if (!(buf[12] == 0xE1 && buf[13] == 0x10)) {
      showStatus("Writing CC...");
      const uint8_t cc[4] = {0xE1, 0x10, 0x3E, 0x00};
      if (!writePage(3, cc)) { okAll = false; break; }
    }

    // ---- ここをあなたのX(Twitter) URLに ----
    String url = "https://x.com/";

    // NDEF生成
    showStatus("Building NDEF...");
    uint8_t ndef[256];
    size_t nlen = buildNdefUrl(url, ndef, sizeof(ndef));
    if (!nlen) { okAll = false; break; }

    // 書き込み
    showStatus("Writing...", url.c_str());

    uint8_t page = 4;
    for (size_t i = 0; i < nlen; i += 4) {
      uint8_t d[4] = {0, 0, 0, 0};
      for (int k = 0; k < 4; k++) if (i + k < nlen) d[k] = ndef[i + k];

      if (!writePage(page, d)) { okAll = false; break; }
      page++;

      // 進捗表示（軽く）
      char prog[32];
      snprintf(prog, sizeof(prog), "page %u", page);
      M5.Display.setCursor(0, 60);
      M5.Display.setTextSize(2);
      M5.Display.println(prog);
      delay(20);
    }
  } while (false);

  // 後処理
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // 結果表示
  if (okAll) showStatus("SUCCESS", "Open with phone");
  else      showStatus("FAILED", "Try again");

  delay(1500);
  showStatus("Ready", "Tap NTAG215");
}
