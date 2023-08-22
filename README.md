# RaMI
Rs-atodashi-MIdi<br>
X68000ZのRS-MIDIを遅延再生させるやつ<br>
<br>
![回路図](https://github.com/yagennankoz/RaMI/blob/main/images/RaMI_CircuitDiagram.jpg)
<br>
X68000ZでRS-MIDIを用いて内蔵音源とMIDIの同時演奏を行うと、MIDIの再生が先行してしまいます。<br>
これを解消するためUARTから入力されたMIDIデータを少し遅延させてMIDI機器に出力します。
