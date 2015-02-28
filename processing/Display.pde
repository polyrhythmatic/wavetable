class Display extends Thread {
  int count = 0;
  boolean countBool = false;

  void waveform() {
    //drawing the waveform

      for (int i = 0; i <=sampleSize-1; i++) {
      int randomizer = (int)random(-2, 2);

      strokeWeight(1);
      stroke(count);
      float x1 = float(i*width/sampleSize);
      float y1 = -(float)interpWaveformY[i]*100 + (height/2);
      float x2 = float((i+1)*width/sampleSize);
      float y2 = -(float)interpWaveformY[i+1]*100+ (height/2);
      line(x1 +randomizer, y1 + randomizer, x2+ randomizer, y2+ randomizer);
    }
    if (count > 210) {
      countBool = true;
    }
    if (count < 140) {
      countBool = false;
    }
    if (countBool == false) {
      count ++;
    }
    if (countBool == true) {
      count --;
    }
  }

  void interfaceLines() {
    for (int i = 1; i <=7; i++) {//drawing the interface lines
      strokeWeight(1);
      stroke(0);
      line(waveSliders.get(i).sliderX, waveSliders.get(i).sliderY, waveSliders.get(i-1).sliderX, waveSliders.get(i-1).sliderY);
    }
    line(0, height/2, waveSliders.get(0).sliderX, waveSliders.get(0).sliderY);
    line(width, height/2, waveSliders.get(7).sliderX, waveSliders.get(7).sliderY);
  }
}
