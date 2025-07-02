<h1 align=center>
  femtolog
</h1>

## ☄ 概要

**femtolog** は、パフォーマンス重視のアプリケーション向けに設計された、超高速・最小オーバーヘッドの非同期ロギングライブラリです。ゼロコスト抽象化、キャッシュアラインされたSPSCキュー、コンパイル時のフォーマット文字列シリアライズを活用しています。

ナノ秒単位のパフォーマンスが求められる最新のC++プロジェクト向けに設計されています。

---

## 📖 目次
- [☄ 概要](#-概要)
- [📖 目次](#-目次)
- [🚀 特徴](#-特徴)
- [📦 使い方](#-使い方)
- [🔄 ワークフロー](#-ワークフロー)
- [📊 ベンチマーク](#-ベンチマーク)
  - [リテラル文字列(フォーマットなし)のログ](#リテラル文字列フォーマットなしのログ)
  - [フォーマットあり文字列のログ](#フォーマットあり文字列のログ)
- [🔧 インストール](#-インストール)
  - [CMakeを使う場合](#cmakeを使う場合)
- [🔌 カスタムシンク](#-カスタムシンク)
  - [✨ 独自シンクの実装](#-独自シンクの実装)
- [🪪 ライセンス](#-ライセンス)
- [❤️ クレジット](#️-クレジット)


## 🚀 特徴

- 🔧 コンパイル時のフォーマット文字列登録
- 🧵 真の非同期ロギングパイプライン
- 🎯 フロントエンドでは動的メモリアロケーションゼロ
- 💾 フォーマットと出力専用のバックエンドワーカースレッド
- ⚡ ベンチマークで `spdlog` や `quill` より高速

---

## 📦 使い方

`femtolog` は [fmtlib](https://github.com/fmtlib/fmt) を使ったメッセージフォーマットに対応しています。

```cpp
#include "femtolog/femtolog.h"

int main() {
  // スレッドローカルなロガーインスタンスを取得
  femtolog::Logger logger = femtolog::Logger::logger();

  // ロガーを初期化し、シンクを登録
  logger.init();
  logger.register_sink<femtolog::StdoutSink<>>();
  logger.register_sink<femtolog::FileSink<>>("");
  logger.level("trace");

  // ログエントリをデキューするバックエンドワーカーを起動
  logger.start_worker();

  std::string username = "pugur";
  float cpu_usage = 42.57;
  bool result = true;
  int error_code = -1;

  // コンパイル時フォーマット文字列でログ出力:
  logger.trace<"Hello {}\n">("World");
  logger.debug<"Hello World wo formatting\n">();
  logger.info<"User \"{}\" logged in.\n">(username);
  logger.warn<"CPU usage is high: {}%\n">(cpu_usage);
  logger.error<"Return value is: {}\n">(result);

  logger.fatal<"Fatal error occured; error code: {}\n">(error_code);

  logger.stop_worker();
  logger.clear_sinks();

  return 0;
}
```

文字列リテラル（`"..."`）はコンパイル時にformat_idとしてハッシュ値が計算されます。引数は生バイトストリームとしてシリアライズされ、非同期でパイプラインを通過します。

## 🔄 ワークフロー

ロギングパイプラインはフロントエンド（スレッドローカルロガー）とバックエンド（ワーカースレッド）で構成されます:
```
logger (フロントエンド)
   |
   |-- serialize(format_id, format_args...) --> [ SPSC Queue ]
                                                      |
                                                      v
                                                  backend_worker (非同期)
                                                  |
                                                  |-- deserialize + formatting
                                                  |
                                                  '--> sink（標準出力、ファイル、カスタムターゲット）
```
このアーキテクチャにより、フォーマット処理をロギングのホットパスから分離し、レイテンシを最小化します。

## 📊 ベンチマーク

以下のベンチマーク結果は [Google Benchmark](https://github.com/google/benchmark) を用い、Intel Core i3 121000F, Linux x86_64, Clang 21, -O3 の環境で測定したものです。
使用したベンチマークコードは [`//src/bench/`](src/bench/) ディレクトリにて確認できます。

### リテラル文字列(フォーマットなし)のログ

| ライブラリ   | レイテンシ中央値 (ns) | スループット (msgs/sec) |
| :----------- | :-------------------- | :---------------------- |
| **femtolog** | **3.48 ns**           | **\~198.9M**            |
| quill        | 23.8 ns               | \~30.7M                 |
| spdlog       | 30.2 ns               | \~18.6M                 |

### フォーマットあり文字列のログ

| ライブラリ   | レイテンシ中央値 (ns) | スループット (msgs/sec) |
| :----------- | :-------------------- | :---------------------- |
| **femtolog** | **10.6 ns**           | **\~63.8M**             |
| quill        | 23.0 ns               | \~29.8M                 |
| spdlog       | 52.2 ns               | \~11.2M                 |


## 🔧 インストール

### CMakeを使う場合

このリポジトリをgitサブモジュールとして追加します:
```bash
git submodule add https://github.com/pugur523/femtolog.git ./femtolog --recursive
```

`femtolog` をサブディレクトリとして追加:

```cmake
add_subdirectory(femtolog)

target_link_libraries(your_target PRIVATE femtolog)
```

ライブラリをインストールする場合:
```cmake
set(INSTALL_FEMTOLOG TRUE)
set(FEMTOLOG_INSTALL_HEADERS TRUE)
add_subdirectory(femtolog)

target_link_libraries(your_target PRIVATE femtolog)
```

## 🔌 カスタムシンク
データベースやネットワークソケット、リングバッファへのログ出力も可能です。

`femtolog` はシンプルなインターフェースでカスタムシンクをサポートしています。

### ✨ 独自シンクの実装
カスタムシンクを定義するには、`SinkBase` を継承し `on_log()` を実装します:
```cpp
#include "femtolog/sinks/sink_base.h"

class MySink : public femtolog::SinkBase {
 public:
  void on_log(const LogEntry& entry, const char* content, std::size_t len) override {
    // ファイル書き込み、ネットワーク送信など
    std::fwrite(content, 1, len, stderr);
  }
};
```
そしてロガーにシンクを登録します:
```cpp
logger.register_sink<MySink>();
```
これだけで、バックエンドから完全にフォーマット済みのログエントリを非同期で受け取ることができます。

## 🪪 ライセンス
`femtolog` は [Apache 2.0 License](LICENSE) の下でライセンスされています。

## ❤️ クレジット

- **[zlib](https://github.com/madler/zlib)**<br/>
  `FileSink` および `JsonLinesSink` で効率的なログファイル圧縮に使用。
- **[GoogleTest (gtest)](https://github.com/google/googletest)**<br/>
  プロジェクト全体の主なユニットテストフレームワーク。
- **[Google Benchmark](https://github.com/google/benchmark)**<br/>
  他のロギングライブラリとの性能比較ベンチマークに使用。
- **[fmtlib](https://github.com/fmtlib/fmt)**<br/>
  すべてのログメッセージレンダリングのフォーマットエンジン。

[pugur](https://github.com/pugur523)が開発しました