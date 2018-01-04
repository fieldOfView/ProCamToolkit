# ofxMapamok

ofxMapamok is an addon that offers the functionality of the [mapamok](https://github.com/YCAMInterlab/ProCamToolkit/wiki/mapamok-%28English%29)
application that is part of [ProCamToolkit](https://github.com/YCAMInterlab/ProCamToolkit), written by Kyle McDonald for YCAMInterlab. It builds on the original ProCamToolkit code and the slightly newer [mapamok repository](https://github.com/YCAMInterlab/mapamok/).

The addon comes with an example named 'mapamok' that has most of the functionality of the original application. It allows you to load a 3d model and then specify some number of points between the model and the corresponding location in the projection. After enough points have been selected, it will solve for the projector location and set the OpenGL viewport to render with the same intrinsics as the projector. For more details about mapamok, see [the original wiki](https://github.com/YCAMInterlab/ProCamToolkit/wiki).

By using the ofxMapamok addon and adding an ofxMapamok object to your application, you can easily load and use the calibration data in
any app openFrameworks application. The ofxMapamok has a single dependency on ofxOpenCV, which is a core addon.

ofxMapamok and ProCamToolkit are available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).

----

*ofxMapamok is derived from ProCamToolkit, which is codeveloped by [YCAM Interlab](http://interlab.ycam.jp/en/).*