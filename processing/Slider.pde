class Slider {
  boolean dragging = false;
  boolean rollover = false;

  int sliderY = height/2;
  int sliderX = 0;

  int mouseDist = 0;

  int sliderRadius = 2;

  Slider(int tempX) {
    sliderX = tempX;
  }

  void display() {
    fill(0);
    noStroke();
    ellipse(sliderX, sliderY, 2*sliderRadius, 2*sliderRadius);
  }

  void clicked(int mX, int mY) {

    mouseDist= int(dist(mX, mY, sliderX, sliderY));

    if (mouseDist <= sliderRadius) {
      dragging= true;
    }
  }

  void drag(int mX, int mY) {
    if (dragging == true) {
      //sliderX = mX;
      sliderY = mY;
    }
  }
  void stopDragging() {
    dragging = false;
  }
  void midiSlider(int mY) {
    sliderY = mY;
  }
}
