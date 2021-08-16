// Copyright 2021 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.mediapipe.examples.hands;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.provider.MediaStore;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import com.google.mediapipe.formats.proto.LandmarkProto.NormalizedLandmark;
import com.google.mediapipe.solutioncore.CameraInput;
import com.google.mediapipe.solutioncore.SolutionGlSurfaceView;
import com.google.mediapipe.solutions.hands.HandLandmark;
import com.google.mediapipe.solutions.hands.Hands;
import com.google.mediapipe.solutions.hands.HandsOptions;
import com.google.mediapipe.solutions.hands.HandsResult;
import java.io.IOException;

/** Main activity of MediaPipe Hands app. */
public class MainActivity extends AppCompatActivity {
  private static final String TAG = "MainActivity";

  private Hands hands;
  private int mode = HandsOptions.STATIC_IMAGE_MODE;
  // Image demo UI and image loader components.
  private Button loadImageButton;
  private ActivityResultLauncher<Intent> imageGetter;
  private HandsResultImageView imageView;

  // Live camera demo UI and camera components.
  private Button startCameraButton;
  private CameraInput cameraInput;
  private SolutionGlSurfaceView<HandsResult> glSurfaceView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    setupStaticImageDemoUiComponents();
    setupLiveDemoUiComponents();
  }

  @Override
  protected void onResume() {
    super.onResume();
    if (mode == HandsOptions.STREAMING_MODE) {
      // Restarts the camera and the opengl surface rendering.
      cameraInput = new CameraInput(this);
      cameraInput.setCameraNewFrameListener(textureFrame -> hands.send(textureFrame));
      glSurfaceView.post(this::startCamera);
      glSurfaceView.setVisibility(View.VISIBLE);
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    if (mode == HandsOptions.STREAMING_MODE) {
      stopLiveDemo();
    }
  }

  /** Sets up the UI components for the static image demo. */
  private void setupStaticImageDemoUiComponents() {
    // The Intent to access gallery and read images as bitmap.
    imageGetter =
        registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(),
            result -> {
              Intent resultIntent = result.getData();
              if (resultIntent != null) {
                if (result.getResultCode() == RESULT_OK) {
                  Bitmap bitmap = null;
                  try {
                    bitmap =
                        MediaStore.Images.Media.getBitmap(
                            this.getContentResolver(), resultIntent.getData());
                  } catch (IOException e) {
                    Log.e(TAG, "Bitmap reading error:" + e);
                  }
                  if (bitmap != null) {
                    hands.send(bitmap);
                  }
                }
              }
            });
    loadImageButton = (Button) findViewById(R.id.button_load_picture);
    loadImageButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            if (mode == HandsOptions.STREAMING_MODE) {
              stopLiveDemo();
            }
            if (hands == null || mode != HandsOptions.STATIC_IMAGE_MODE) {
              setupStaticImageModePipeline();
            }
            // Reads images from gallery.
            Intent gallery =
                new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.INTERNAL_CONTENT_URI);
            imageGetter.launch(gallery);
          }
        });
    imageView = new HandsResultImageView(this);
  }

  /** The core MediaPipe Hands setup workflow for its static image mode. */
  private void setupStaticImageModePipeline() {
    // Initializes a new MediaPipe Hands instance in the static image mode.
    mode = HandsOptions.STATIC_IMAGE_MODE;
    if (hands != null) {
      hands.close();
    }
    hands = new Hands(this, HandsOptions.builder().setMode(mode).build());

    // Connects MediaPipe Hands to the user-defined HandsResultImageView.
    hands.setResultListener(
        handsResult -> {
          logWristLandmark(handsResult, /*showPixelValues=*/ true);
          runOnUiThread(() -> imageView.setHandsResult(handsResult));
        });
    hands.setErrorListener((message, e) -> Log.e(TAG, "MediaPipe hands error:" + message));

    // Updates the preview layout.
    FrameLayout frameLayout = (FrameLayout) findViewById(R.id.preview_display_layout);
    frameLayout.removeAllViewsInLayout();
    imageView.setImageDrawable(null);
    frameLayout.addView(imageView);
    imageView.setVisibility(View.VISIBLE);
  }

  /** Sets up the UI components for the live demo with camera input. */
  private void setupLiveDemoUiComponents() {
    startCameraButton = (Button) findViewById(R.id.button_start_camera);
    startCameraButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            if (hands == null || mode != HandsOptions.STREAMING_MODE) {
              setupStreamingModePipeline();
            }
          }
        });
  }

  /** The core MediaPipe Hands setup workflow for its streaming mode. */
  private void setupStreamingModePipeline() {
    // Initializes a new MediaPipe Hands instance in the streaming mode.
    mode = HandsOptions.STREAMING_MODE;
    if (hands != null) {
      hands.close();
    }
    hands = new Hands(this, HandsOptions.builder().setMode(mode).build());
    hands.setErrorListener((message, e) -> Log.e(TAG, "MediaPipe hands error:" + message));

    // Initializes a new CameraInput instance and connects it to MediaPipe Hands.
    cameraInput = new CameraInput(this);
    cameraInput.setCameraNewFrameListener(textureFrame -> hands.send(textureFrame));

    // Initalizes a new Gl surface view with a user-defined HandsResultGlRenderer.
    glSurfaceView =
        new SolutionGlSurfaceView<>(this, hands.getGlContext(), hands.getGlMajorVersion());
    glSurfaceView.setSolutionResultRenderer(new HandsResultGlRenderer());
    glSurfaceView.setRenderInputImage(true);
    hands.setResultListener(
        handsResult -> {
          logWristLandmark(handsResult, /*showPixelValues=*/ false);
          glSurfaceView.setRenderData(handsResult);
          glSurfaceView.requestRender();
        });

    // The runnable to start camera after the gl surface view is attached.
    glSurfaceView.post(this::startCamera);

    // Updates the preview layout.
    FrameLayout frameLayout = (FrameLayout) findViewById(R.id.preview_display_layout);
    imageView.setVisibility(View.GONE);
    frameLayout.removeAllViewsInLayout();
    frameLayout.addView(glSurfaceView);
    glSurfaceView.setVisibility(View.VISIBLE);
    frameLayout.requestLayout();
  }

  private void startCamera() {
    cameraInput.start(
        this,
        hands.getGlContext(),
        CameraInput.CameraFacing.FRONT,
        glSurfaceView.getWidth(),
        glSurfaceView.getHeight());
  }

  private void stopLiveDemo() {
    if (cameraInput != null) {
      cameraInput.stop();
    }
    if (glSurfaceView != null) {
      glSurfaceView.setVisibility(View.GONE);
    }
  }

  private void logWristLandmark(HandsResult result, boolean showPixelValues) {
    NormalizedLandmark wristLandmark = Hands.getHandLandmark(result, 0, HandLandmark.WRIST);
    // For Bitmaps, show the pixel values. For texture inputs, show the normoralized cooridanates.
    if (showPixelValues) {
      int width = result.inputBitmap().getWidth();
      int height = result.inputBitmap().getHeight();
      Log.i(
          TAG,
          "MediaPipe Hand wrist coordinates (pixel values): x= "
              + wristLandmark.getX() * width
              + " y="
              + wristLandmark.getY() * height);
    } else {
      Log.i(
          TAG,
          "MediaPipe Hand wrist normalized coordinates (value range: [0, 1]): x= "
              + wristLandmark.getX()
              + " y="
              + wristLandmark.getY());
    }
  }
}
