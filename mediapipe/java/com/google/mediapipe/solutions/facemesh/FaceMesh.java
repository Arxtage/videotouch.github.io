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

package com.google.mediapipe.solutions.facemesh;

import android.content.Context;
import com.google.common.collect.ImmutableList;
import com.google.mediapipe.formats.proto.LandmarkProto.NormalizedLandmarkList;
import com.google.mediapipe.framework.MediaPipeException;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.solutioncore.ErrorListener;
import com.google.mediapipe.solutioncore.ImageSolutionBase;
import com.google.mediapipe.solutioncore.OutputHandler;
import com.google.mediapipe.solutioncore.ResultListener;
import com.google.mediapipe.solutioncore.SolutionInfo;
import java.util.HashMap;
import java.util.Map;
import javax.annotation.Nullable;

/**
 * MediaPipe FaceMesh Solution API.
 *
 * <p>MediaPipe FaceMesh processes a {@link TextureFrame} or a {@link Bitmap} and returns the face
 * landmarks of each detected face. Please refer to
 * https://solutions.mediapipe.dev/face_mesh#android-solution-api for usage examples.
 */
public class FaceMesh extends ImageSolutionBase {
  private static final String TAG = "FaceMesh";

  private static final String NUM_FACES = "num_faces";
  private static final String GPU_GRAPH_NAME = "face_landmark_front_gpu_image.binarypb";
  private static final String CPU_GRAPH_NAME = "face_landmark_front_cpu_image.binarypb";
  private static final String IMAGE_INPUT_STREAM = "image";
  private static final ImmutableList<String> OUTPUT_STREAMS =
      ImmutableList.of("multi_face_landmarks", "image");
  private static final int LANDMARKS_INDEX = 0;
  private static final int INPUT_IMAGE_INDEX = 1;
  private final OutputHandler<FaceMeshResult> graphOutputHandler;

  /**
   * Initializes MediaPipe FaceMesh solution.
   *
   * @param context an Android {@link Context}.
   * @param options the configuration options defined in {@link FaceMeshOptions}.
   */
  public FaceMesh(Context context, FaceMeshOptions options) {
    graphOutputHandler = new OutputHandler<>();
    graphOutputHandler.setOutputConverter(
        packets -> {
          FaceMeshResult.Builder faceMeshResultBuilder = FaceMeshResult.builder();
          try {
            faceMeshResultBuilder.setMultiFaceLandmarks(
                getProtoVector(packets.get(LANDMARKS_INDEX), NormalizedLandmarkList.parser()));
          } catch (MediaPipeException e) {
            throwException("Error occurs when getting MediaPipe facemesh landmarks. ", e);
          }
          return faceMeshResultBuilder
              .setImagePacket(packets.get(INPUT_IMAGE_INDEX))
              .setTimestamp(
                  staticImageMode ? Long.MIN_VALUE : packets.get(INPUT_IMAGE_INDEX).getTimestamp())
              .build();
        });

    SolutionInfo solutionInfo =
        SolutionInfo.builder()
            .setBinaryGraphPath(options.runOnGpu() ? GPU_GRAPH_NAME : CPU_GRAPH_NAME)
            .setImageInputStreamName(IMAGE_INPUT_STREAM)
            .setOutputStreamNames(OUTPUT_STREAMS)
            .setStaticImageMode(options.mode() == FaceMeshOptions.STATIC_IMAGE_MODE)
            .build();

    initialize(context, solutionInfo, graphOutputHandler);
    Map<String, Packet> inputSidePackets = new HashMap<>();
    inputSidePackets.put(NUM_FACES, packetCreator.createInt32(options.maxNumFaces()));
    start(inputSidePackets);
  }

  /**
   * Sets a callback to be invoked when the FaceMeshResults become available.
   *
   * @param listener the {@link ResultListener} callback.
   */
  public void setResultListener(ResultListener<FaceMeshResult> listener) {
    this.graphOutputHandler.setResultListener(listener);
  }

  /**
   * Sets a callback to be invoked when the FaceMesh solution throws errors.
   *
   * @param listener the {@link ErrorListener} callback.
   */
  public void setErrorListener(@Nullable ErrorListener listener) {
    this.graphOutputHandler.setErrorListener(listener);
    this.errorListener = listener;
  }
}
