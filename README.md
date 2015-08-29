# tracking
People Detection and Tracking by OpenCV 3.0

## 必要な環境

* C++11
* OpenCV 3.0
* boost (ファイル読み込みのみ使用）

## ビルド
```
$ make
```
で、実行ファイルができます。

## 使い方

入力映像のフレーム画像が入ったディレクトリを用意します。
```
$ tree videodir

View_001/
├── frame_0000.jpg
├── frame_0001.jpg
...
├── frame_0793.jpg
└── frame_0794.jpg

0 directories, 795 files
```

以下のコマンドで実行可能です。
```
$ ./tracking videodir
```

動画を入力にしたい場合は、入力部分を`cv::VideoCapture`などで実装しなおしてください。

## 動作例

PETS2009 S2.L1 View01 での動作例は[こちら](http://youtu.be/qO6xz-vTko4)。

