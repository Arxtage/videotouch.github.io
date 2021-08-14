## Video-Touch
<p align="center">
  <img src="https://github.com/Arxtage/videotouch.github.io/blob/gh-pages/media/gif0-2.gif?raw=true">
</p>

### Description
**Video-Touch: Remote Robot Control by DNN-based Gesture Recognition**<br/>

We present Video-Touch, a breakthrough technology for multi-user and real-time control of robot by DNN-based gesture recognition. The users can have a video conferencing in a digital world and at the same time to perform dexterous manipulations with tangible objects by remote robot. As the scenario, we proposed the remote robotic COVID-19 test Laboratory to substitute medical assistant working in protective gear in close proximity with infected cells and to considerably reduce the time to receive the test results. The proposed technology suggests a new type of reality, where multi-users can jointly interact with remote object (e.g. make a new building design, joint cooking in robotic kitchen, etc), and discuss/modify the results at the same time.


<p align="center">
  <img width="1566" alt="gif1-3" src="https://user-images.githubusercontent.com/44577835/129403308-b734b6d5-67f1-44d7-b2b8-53c7c7eee3ee.png">
</p>

### System Overview

We were wondering if it is even possible to control a robot remotely using only your own hands - without any additional devices like gloves or a joystick - not suffering from a significant delay. We decided to use computer vision to recognize movements in real-time and instantly pass them to the robot. Thanks to MediaPipe now it is possible.

Our system looks as follows:
1. Video conference application gets a webcam video on the user device and sends it to the robot computer (“server”);
2. User webcam video stream is being captured on the robot's computer display via OBS virtual camera tool;
3. The recognition module reads user movements and gestures with the help of [MediaPipe](https://mediapipe.dev/) and sends it to the next module via [ZeroMQ](https://zeromq.org/);
4. The robotic arm and its gripper are being controlled from Python, given the motion capture data.
<p align="center">
  <img width="500" alt="gif1-3" src="https://user-images.githubusercontent.com/44577835/129403360-d7b8e241-338f-4eb4-8ead-b7d7162ef4d7.PNG">
</p>

### Live Demos

This project has had a great reception not only in  ​robotics but also in other areas such as life sciences, art, and medicine. So much so that it has been featured in various conferences, festivals, and television channels.

**1. SIGGRAPH Asia 2020**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/44577835/129399087-6a58ddb3-f346-44d2-9af7-50bf4dd123db.PNG)](https://www.youtube.com/watch?v=F4X4jJwDBy4)
<br/>
<br/>
**2. Russia 24 TV Channel**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/44577835/129400595-b11420fd-299f-40ae-a092-bba288d51e55.PNG)](https://www.youtube.com/watch?v=5p9MlKYu3RA&t=488s)
<br/>
<br/>
**2. Russia 1 TV Channel**<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/44577835/129442633-21619542-a8dd-4de0-97ef-958341509dc7.png)](https://avto.vesti.ru/video/2277992)
<br/>
<br/>
**3. PANGARDENIA ARS ELECTRONICA 2020** Festival for Art, Technology & Society Saint-Petersburg<br/>
<br/>
[![IMAGE_ALT](https://user-images.githubusercontent.com/44577835/129401928-bd267ca4-61bf-43da-9db4-197ed88dde9a.PNG)](https://youtu.be/i6dYyRQg-iM?t=4813)

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



You can use the [editor on GitHub](https://github.com/Arxtage/videotouch.github.io/edit/gh-pages/index.md) to maintain and preview the content for your website in Markdown files.

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [GitHub Flavored Markdown](https://guides.github.com/features/mastering-markdown/).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/Arxtage/videotouch.github.io/settings/pages). The name of this theme is saved in the Jekyll `_config.yml` configuration file.


