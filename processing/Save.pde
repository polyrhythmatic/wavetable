class Save {

  void saveValues(int buttonNum) {
    println("save");
    String[] data = new String[18];
    for (int i = 0; i <10; i++) {
      data[i] = ""+ (int)map((float)dWaveformY[i], -1, 1, 1023, 0);
    }
    data[10] = "" + attack;
    data[11] = "" + decay;
    data[12] = "" + sustain;
    data[13] = "" + release;

    data[14] = "" + splinePercent;
    data[15] = "" + linearPercent;
    data[16] = "" + squarePercent;
    data[17] = "" + sampleSize;

    saveStrings("userSettings"+buttonNum+".txt", data);
  }

  void loadValues(int buttonNum) {
    String[] data = loadStrings("userSettings"+buttonNum+".txt");
    // for (int i = 0; i <10; i++) {

    //dWaveformY[i] = Double.parseDouble(data[i]);
    //println(Double.parseDouble(data[i]));
    //println(dWaveformY[i]);
    // }
    
    slidersOut ="2,";
    
    for (int i = 1; i < 9; i++) {
      slidersOut += data[i] +",";
      //   waveSliders.get(i-1).sliderY = int(map(float(data[i]), -1, 1, height, 0));
      //   waveSliders.get(i-1).display();
    }
    slidersOut += '$';
    slidersOut += '\n';
    //myPort.write(slidersOut);
    
    // attack = Float.parseFloat(data[10]);
    // decay = Float.parseFloat(data[11]);
    // sustain = Float.parseFloat(data[12]);
    // release = Float.parseFloat(data[13]);

    // splinePercent = Float.parseFloat(data[14]);
    // linearPercent = Float.parseFloat(data[15]);
    // squarePercent = Float.parseFloat(data[16]);

    // sampleSizeTemp = int(data[17]);

    // adsrSliders.get(0).sliderY = int(map(attack, 0.01, 4, height, 0));
    // adsrSliders.get(1).sliderY = int(map(decay, 0.0001, 10, height, 0));
    // adsrSliders.get(2).sliderY = int(map(sustain, 0.0001, 1, height, 0));
    // adsrSliders.get(3).sliderY = int(map(release, 0.0001, 10, height, 0));

    // for (int i = 0; i < adsrSliders.size (); i++) {
    //   adsrSliders.get(i).display();
    // }
  }
}
