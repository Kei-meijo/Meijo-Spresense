# 成果物 "山菜自動判別システム"

## 機能  
装置を首にかけた状態でしゃがむことで目の前にある山菜を判別し,それが有毒であれば音声により警告を行う.

## 使用方法 
DNNにより山菜判別を行うSpresenseの設定
1. Spresense1/SDフォルダ内の中身を1台目のSDカードに書き込む
2. Spresense1/src/DNN.inoをArduino IDEにて開きSpresenseに書き込む

DNNの結果を音声出力するSpresenseの設定
1. https://developer.sony.com/develop/spresense/docs/arduino_tutorials_ja.html#_mp3_player に基づいてmp3音源が再生可能な状態とする
2. Spresense2/SDフォルダ内の中身を2台目のSDカードに書き込む
3. Spresense2/src/music.inoをArduino IDEにて開きSpresenseに書き込む
 
接続方法
Spresenseのデジタルピン2番同士とGND同士を接続する

## プログラムの説明  
1.Spresense1/src/DNN.ino
カメラにて画像を取り込み,DNNを行うメインとなるプログラム.

2.Spresense2/src/music.ino
DNNの結果を受け取り,音声を再生するプログラム.
メモリの関係上同時にDNNと音声再生ができなかったため,Spresense2台にて実装を行った.

3.Spresense1/SD/model.nnb
DNNのモデルファイル.今回の対象ではフキノトウとハシリドコロの2種にて分類を行う.それぞれ30枚ずつ画像を用意し学習を行った.

4.Spresense2/SD/sansai.mp3
DNNによる判別によって有毒植物であった場合に再生される警告音声.

## SysMLモデル  


##  プログラム ライセンス 
MIT  

