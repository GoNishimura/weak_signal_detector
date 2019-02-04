# weak_signal_detector
Arduino programs for visual light communication that can read weak signals under random noises

<img src="https://github.com/GoNishimura/images/blob/master/weak_signal_detector.gif">
<img src="https://github.com/GoNishimura/images/blob/master/weak_sig_circuit.png">
<img src="https://github.com/GoNishimura/images/blob/master/IMG_3313.jpg">


再現手順
<ol>
  <li>回路図に従って、ArduinoとLEDをセットアップする。この時、LEDの頭同士を向き合わせると信号が強くなる。</li>
  <li>このレポジトリのコードを受信機（weakSigRx）、送信機（weakSigTx）の順番にArduinoで実行させる。必要なライブラリをArduinoのlibrariesフォルダに入れることをお忘れなく。受信機の立ち上げには若干時間がかかるので、受信機側のプログラムを実行中の時、シリアルモニタで"The thresh is: "という表示が出てから送信機を立ち上げ、シリアルモニタでは引き続き受信機からの出力を表示させ続けると良い。</li>
  <li>色々と表示される中で、いくつかの空行の後に"Found key char"という表示が受信機のシリアルモニタで表示されれば、正しく情報を認識できたこととなる。これを応用すれば、可視光でシリアル通信を行うことが可能となる。</li>
</ol>


visual light communicationについての細かい話は、<a href="https://github.com/jpiat/arduino">このレポジトリ</a>が非常に詳しく解説してくれている上、プログラムも大いに参考になる。

このプログラムの簡単な原理とかは：https://github.com/GoNishimura/images/blob/master/weak_sig_info.pdf
