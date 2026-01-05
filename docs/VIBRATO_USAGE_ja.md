# Vibratoサポートの使用方法

MZ-IMEjaのVibrato形態素解析エンジン統合に関するドキュメントです。

## 概要

Vibratoは高性能な形態素解析エンジンで、MZ-IMEjaに統合することで以下の利点があります：

- より高速なかな漢字変換
- より正確な形態素解析
- コンパクトな辞書形式

## 前提条件

- Rust 1.70以降
- Cargo
- CMake 3.10以降
- C++コンパイラ（Visual Studio、GCC、またはClang）

## ビルド手順

### 1. Vibrato C APIバインディングのビルド

Vibrato C APIバインディングは自動的にビルドされますが、手動でビルドすることもできます：

```bash
cd third_party/vibrato-c
cargo build --release
```

### 2. Vibratoサポートを有効にしてMZ-IMEjaをビルド

```bash
mkdir build && cd build
cmake .. -DUSE_VIBRATO=ON
cmake --build .
```

### 3. 辞書の準備

Vibrato用の辞書を準備します：

```bash
# MeCab IPA辞書をダウンロードしてVibrato形式に変換
bash tools/convert_mecab_dict.sh
```

これにより、`dicts/vibrato/ipadic.vibrato` に辞書ファイルが作成されます。

### 4. 辞書のインストール

ビルドしたIMEと一緒に辞書ファイルを配置します：

```
mzimeja.ime
dicts/
  vibrato/
    ipadic.vibrato
```

## 設定

### 辞書パスの設定

Vibratoエンジンは以下の順序で辞書を検索します：

1. レジストリで指定されたパス
2. `%PROGRAMFILES%\mzimeja\dicts\vibrato\ipadic.vibrato`
3. IME実行ファイルと同じディレクトリ内の `dicts\vibrato\ipadic.vibrato`

### Vibratoの無効化

Vibratoサポートなしでビルドする場合：

```bash
cmake .. -DUSE_VIBRATO=OFF
cmake --build .
```

## フォールバック動作

Vibratoが有効だが辞書が見つからない、または初期化に失敗した場合、MZ-IMEjaは自動的にレガシー変換エンジンにフォールバックします。これにより、IMEは常に動作することが保証されます。

## トラブルシューティング

### "Vibrato library not found"エラー

Rustがインストールされていることを確認してください：

```bash
rustc --version
cargo --version
```

### 辞書の読み込みエラー

- 辞書ファイルのパスが正しいか確認
- 辞書ファイルの読み取り権限があるか確認
- 辞書ファイルが破損していないか確認

### ビルドエラー

- CMakeのバージョンが3.10以降であることを確認
- C++コンパイラが正しくインストールされているか確認

## パフォーマンス

Vibratoを使用すると、レガシーエンジンと比較して以下の改善が期待されます：

- 変換速度: 約3倍高速（目標値）
- メモリ使用量: 約50%削減（目標値）
- 辞書サイズ: 約50%削減（目標値）

※これらは推定値であり、実際のパフォーマンスは実装完了後に測定されます。

## 技術的詳細

### アーキテクチャ

```
MZ-IME (C++)
    ↓
VibratoEngine (C++)
    ↓
vibrato-c (Rust FFI)
    ↓
Vibrato (Rust)
```

### 品詞マッピング

Vibratoが使用するMeCab IPA辞書の品詞は、MZ-IMEja内部の品詞体系に自動的にマッピングされます。

主なマッピング：

| MeCab品詞 | MZ-IMEja品詞 |
|-----------|--------------|
| 名詞 | HB_MEISHI |
| 動詞 | HB_GODAN_DOUSHI / HB_ICHIDAN_DOUSHI |
| 形容詞 | HB_IKEIYOUSHI |
| 助詞,格助詞 | HB_KAKU_JOSHI |
| 助動詞 | HB_JODOUSHI |

## ライセンス

- MZ-IMEja: GPLv3
- Vibrato: MIT OR Apache-2.0

Vibratoのライセンスはmz-imejaのGPLv3と互換性があります。
