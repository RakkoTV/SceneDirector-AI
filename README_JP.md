# SceneDirector-AI 詳明書

SceneDirector-AI は、AI による予測機能を備えたスマートシーン切り替えプラグインです。

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-GPL--3.0-green)

## 機能

- **複数の切り替えモード**
  - ウィンドウフォーカス検出
  - オーディオレベルトリガー
  - 時間ベースのスケジュール
  - AI 予測

- **WebSocket API**
  - 外部制御可能
  - JSON メッセージ形式

## インストール

1. リリースから `SceneDirectorAI.dll` をダウンロード
2. `obs-plugins/64bit/` にコピー
3. OBS Studio を再起動

## 設定

| 設定 | 説明 | デフォルト |
|------|------|----------|
| 切り替えモード | モード選択 | ウィンドウ |
| 予測を有効化 | AI 予測を使用 | オン |
| WebSocket | ポート番号 | 8080 |

## ビルド

[ビルドガイド](docs/BUILD.md) を参照してください

## 寄付

[![Donate](https://img.shields.io/badge/PayPal-Donate-blue)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=ramiro.silva.1993%40gmail%2ecom&lc=US&item_name=Support+Open+Source+Development&currency_code=USD)

## コネクト

- [GitHub](https://github.com/RakkoTV) ⭐
- [Twitch](https://www.twitch.tv/RakkoTech) 👥

## ライセンス

GPL-3.0

---

[RakkoTV](https://github.com/RakkoTV) が ❤️ で作成
