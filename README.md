# Inverted-Pendulum
## 概要
[サクッと作る倒立振子](https://www.hirotakaster.com/weblog/倒立振子をサクッと作る/)を参考に倒立振子を作る.  
cppで実装したソースコードはsrcにあるが動かない.  
arduinoのIDEで動かす場合は[sketch_oct17a](https://github.com/NanaseSAITOH/-Inverted-pendulum/tree/develop/sketch_oct17a)フォルダを参照されたい.  

## 注意点

MPU 6050を使わないと該当プログラムは動かない.

## モータPINについて
### IN1,IN2
|MOTOR_IN1|MOTOR_IN2|モータの回転|
|---------|---------|----------|
|LOW|HIGH|反時計回り|
|HIGH|LOW|時計回り|
|LOW|LOW|停止|

### MOTOR_PWM
モータの回転速度を制御するためのピン．
Arduinoのアナログ(PWM)ピンにつなぐ．
Arduinoでは0～255の値を指定できる．
値を大きくすると大きな電圧がかかるので回転が速くなる.

### 参考にしたサイト

[サクッと作る倒立振子](https://www.hirotakaster.com/weblog/倒立振子をサクッと作る/)
