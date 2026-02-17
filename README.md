# ntag215-x-url-writer

M5StickC と MFRC522 (I2C接続) を使用して、NTAG215 NFCタグに X (旧Twitter) のURLを書き込むプロジェクトです。

## 機能

- NTAG215 カード/タグの検出
- NDEFメッセージ (URIレコード) の作成
- 指定したURL (デフォルトは `https://x.com/`) の書き込み
- M5StickC の画面にステータスを表示

## ハードウェア

- **M5StickC** (または M5Stack 互換機)
- **MFRC522 RFID モジュール** (I2C 対応のもの)
- **NTAG215** カードまたはステッカー

## 配線 (Wiring)

M5StickC の Grove ポート (I2C) を使用する場合のデフォルト設定です。

| MFRC522 | M5StickC (Grove) | Pin (GPIO) |
| :--- | :--- | :--- |
| SDA | SDA | 32 |
| SCL | SCL | 33 |
| VCC | 5V / 3.3V | (モジュールに合わせて接続) |
| GND | GND | GND |
| IRQ | - | 未使用 (コード上はピン0指定) |
| RST | - | 未使用 (コード上はリセットピン指定なし) |

※ MFRC522ライブラリの初期化コード: `MFRC522_I2C rfid(0x28, 0, &Wire);` (アドレス 0x28)

## ソフトウェア依存関係

PlatformIO で管理されています。

- `m5stack/M5Unified`
- `kkloesener/MFRC522_I2C`

## 使い方

1. PlatformIO でプロジェクトをビルドし、M5StickC に書き込みます。
2. M5StickC が起動すると "Ready / Tap NTAG215" と表示されます。
3. NTAG215 タグをリーダーにかざします。
4. 自動的にタグの内容が読み取られ、URL情報が書き込まれます。
5. 書き込みが成功すると "SUCCESS / Open with phone" と表示されます。
6. スマートフォン等でタグを読み取ると、書き込んだURLが開きます。

## URLの変更

`src/main.cpp` の以下の行を変更することで、書き込むURLを変更できます。

```cpp
// ---- ここをあなたのX(Twitter) URLに ----
String url = "https://x.com/";
```
