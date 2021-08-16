# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])

package(default_visibility = ["//mediapipe/examples:__subpackages__"])

cc_import(
  name = "zmq",
  hdrs = ["zmq.h", "zmq.hpp"],
  static_library = "libzmq.a",
)

cc_binary(
    name = "hand_tracking_tflite",
    deps = [
        "//mediapipe/examples/desktop:simple_run_graph_main",
        "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
    ],
)

# cc_binary(
#     name = "hand_tracking_cpu",
#     deps = [
#         "//mediapipe/examples/desktop:demo_run_graph_main",
#         "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
#     ],
# )

cc_binary(
    name = "hand_tracking_cpu",
    srcs = ["hand_tracking_cpu_main.cc"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
        "//mediapipe/calculators/util:rect_to_render_data_calculator",
        "//mediapipe/framework/formats:rect_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "//mediapipe/framework/formats:classification_cc_proto",
        ":zmq",
    ],
)

# Linux only
cc_binary(
    name = "hand_tracking_gpu",
    deps = [
        "//mediapipe/examples/desktop:demo_run_graph_main_gpu",
        "//mediapipe/graphs/hand_tracking:mobile_calculators",
    ],
)
