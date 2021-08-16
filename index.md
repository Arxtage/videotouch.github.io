## Video-Touch
<p align="center">
  <img src="https://github.com/Arxtage/videotouch.github.io/blob/gh-pages/media/gif0-2.gif?raw=true">
</p>

### Description
**Video-Touch: Remote Robot Control by DNN-based Gesture Recognition**<br/>

We present Video-Touch, a breakthrough technology for multi-user and real-time control of robot by DNN-based gesture recognition. The users can have a video conferencing in a digital world and at the same time to perform dexterous manipulations with tangible objects by remote robot. As the scenario, we proposed the remote robotic COVID-19 test Laboratory to substitute medical assistant working in protective gear in close proximity with infected cells and to considerably reduce the time to receive the test results. The proposed technology suggests a new type of reality, where multi-users can jointly interact with remote object (e.g. make a new building design, joint cooking in robotic kitchen, etc), and discuss/modify the results at the same time.


<p align="center">
  <img width="1561" alt="video_touch_scheme" src="https://user-images.githubusercontent.com/17500203/129458733-67a46a4c-68aa-4005-b8a6-a493d42d04e0.png">
</p>

### System Overview

We were wondering if it is even possible to control a robot remotely using only your own hands - without any additional devices like gloves or a joystick - not suffering from a significant delay. We decided to use computer vision to recognize movements in real-time and instantly pass them to the robot. Thanks to MediaPipe now it is possible.

Our system looks as follows:
1. Video conference application gets a webcam video on the user device and sends it to the robot computer (“server”);
2. User webcam video stream is being captured on the robot's computer display via OBS virtual camera tool;
3. The recognition module reads user movements and gestures with the help of [MediaPipe](https://mediapipe.dev/) and sends it to the next module via [ZeroMQ](https://zeromq.org/);
4. The robotic arm and its gripper are being controlled from Python, given the motion capture data.
<p align="center">
  <img width="500" alt="robotic-arm" src="https://user-images.githubusercontent.com/44577835/129403360-d7b8e241-338f-4eb4-8ead-b7d7162ef4d7.PNG">
</p>

### Live Demos

This project has had a great reception not only in  ​robotics but also in other areas such as life sciences, art, and medicine. So much so that it has been featured in various conferences, festivals, and television channels.

**1. SIGGRAPH Asia 2020**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/31474005/129532935-5f21db29-e4f4-4a3d-b34d-630a271d08f1.png)](https://www.youtube.com/watch?v=F4X4jJwDBy4)
<br/>
<br/>
**2. Russia 24 TV Channel**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/31474005/129535009-f118bfa6-a66c-44b9-b260-c8cc5578001a.png)](https://www.youtube.com/watch?v=5p9MlKYu3RA&t=488s)
<br/>
<br/>
**3. Russia 1 TV Channel**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/44577835/129442633-21619542-a8dd-4de0-97ef-958341509dc7.png)](https://avto.vesti.ru/video/2277992)
<br/>
<br/>
**4. PANGARDENIA ARS ELECTRONICA 2020** Festival for Art, Technology & Society Saint-Petersburg<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/31474005/129534618-6c9160f5-1510-47dc-8745-74237d4b8bc8.png)](https://youtu.be/i6dYyRQg-iM?t=4813)

### Authors
- Ilya Zakharkin, Skolkovo Institute of Science and Technology, Moscow Institute of Physics and Technology (State University), Russia
- Arman Tsaturyan, Skolkovo Institute of Science and Technology, Moscow Institute of Physics and Technology (State University), Russia
- Miguel Altamirano Cabrera, Skolkovo Institute of Science and Technology, Russia
- Jonathan Tirado, Skolkovo Institute of Science and Technology, Russia
- Dzmitry Tsetserukou, Skolkovo Institute of Science and Technology, Russia


### Citation
**ZoomTouch: Multi-User Remote Robot Control in Zoom by DNN-based Gesture Recognition**<br/>
Ilya Zakharkin, Arman Tsaturyan, Miguel Altamirano Cabrera, Jonathan Tirado and Dzmitry Tsetserukou<br/>
in SIGGRAPH Asia 2020 Emerging Technologies<br/>
[arXiv](https://arxiv.org/abs/2011.03845)
[ACM Procedings](https://dl.acm.org/doi/10.1145/3415255.3422892)
[Siggraph Asia 2020](https://sa2020.siggraph.org/en/attend/emerging-technologies/session_slot/15/6)


### BibTeX
```markdown
@article{zakharkin2020zoomtouch,
  title={ZoomTouch: Multi-User Remote Robot Control in Zoom by DNN-based Gesture Recognition},
  author={Zakharkin, Ilya and Tsaturyan, Arman and Altamirano-Cabrera, Miguel and Tirado, Jonathan and Tsetserukou, Dzmitry},
  booktitle = {SIGGRAPH Asia 2020 Emerging Technologies},
  year={2020}
}
```
### Support or Contact

Skolkovo Institute of Science and Technology,  Bolshoy Boulevard 30, bld. 1, Moscow, Russia 121205 <br/>
