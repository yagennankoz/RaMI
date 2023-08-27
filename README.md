# RaMI
Rs-atodashi-MIdi<br>
X68000ZのRS-MIDIを遅延再生させるやつ<br>
<br>
X68000ZでRS-MIDIを用いて内蔵音源とMIDIの同時演奏を行うと、MIDIの再生が先行してしまいます。<br>
これを解消するためUARTから入力されたMIDIデータを少し遅延させてMIDI機器に出力します。<br>
<br>
### 用意するもの
・RaspberryPi pico<br>
・X68000Z UARTと接続するためのコネクタかケーブル<br>
・プッシュスイッチ 2個<br>
・抵抗 10Ω、33Ω各1個<br>
・3.5mmステレオジャック(またはDIN5Pのメス)<br>
<br>
### 回路図
![回路図](https://github.com/yagennankoz/RaMI/blob/main/images/RaMI_CircuitDiagram.jpg)
<br>
### 完成イメージ
![完成イメージ](https://github.com/yagennankoz/RaMI/blob/main/images/RaMI_Board.JPG)
<br>
![完成イメージ](https://github.com/yagennankoz/RaMI/blob/main/images/RaMI_case.JPG)
<br>
<br>
### インストール方法(Windowsの場合)
・PCにUSBケーブルを繋いでください<br>
・picoのBOOTSELボタンを押しながらpicoのUSBにケーブルを接続してください。<br>
・Winの場合、picoのフォルダがエクスプローラで立ち上がってきますので、そこにRaMI.uf2 をコピーして下さい。<br>
・ファイルのコピーが完了するとpicoが再起動し、RaMIが動き始めます<br>
win以外での方法は分からないので、お手数ですが各自調べてください。<br>
<br>
### 通常の起動方法
・UARTを繋ぐ<br>
・MIDIケーブルをつなぐ<br>
・picoにUSB電源を繋ぐ<br>
あとはZMUSICRS等でお好きな曲を再生してみて下さい<br>
<br>
### RaMIの操作方法
・二つのボタンで遅延時間の増減ができます。<br>
・ボタンを押したタイミングでその時の遅延時間を保存しますので、次回起動時はその遅延時間で起動します。<br>
・USB UARTに遅延変更時の設定時間をログ出力しているので、シリアル経由でPCで現在の設定を確認することもできます。<br>
