// Copyright 2019 The MediaPipe Authors.
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
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <sys/time.h>
#include <sstream> 

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/classification.pb.h"

#include "zmq.hpp"

// using namespace std::chrono;

constexpr char kWindowName[] = "MediaPipe";
constexpr char kCalculatorGraphConfigFile[] =
    "mediapipe/graphs/hand_tracking/hand_tracking_desktop_live.pbtxt";
// Input and output streams.
constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";
constexpr char kHandednessStream[] = "handedness";
constexpr char kHandLandmarksStream[] = "landmarks";
constexpr char kScaledLandmarksStream[] = "scaled_landmarks";
constexpr char kHandRectsStream[] = "multi_hand_rects";
constexpr char kHandGesturesStream[] = "hand_gestures";


ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");
ABSL_FLAG(int, cam_id, 0,
          "Camera device ID.");


void PrintLandmarks(
    const ::mediapipe::NormalizedLandmarkList& hand_landmarks,
    std::ostream& out_stream,
    float scale_x, float scale_y, float scale_z) {
  out_stream << "[";
  for (int i = 0; i < hand_landmarks.landmark_size(); ++i) {
    const auto& landmark = hand_landmarks.landmark(i);
    out_stream << "[" << landmark.x() * scale_x << " " 
                     << landmark.y() * scale_y << " "
                     << landmark.z() * scale_z << "]";
    if (i < hand_landmarks.landmark_size()-1) {
       out_stream << std::endl;
    }
  }
  out_stream << "]" << std::endl;
}


absl::Status RunMPPGraph(
    std::unique_ptr<::mediapipe::CalculatorGraph> graph) {

  LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();
  if (load_video) {
    capture.open(absl::GetFlag(FLAGS_input_video_path));
  } else {
    capture.open(absl::GetFlag(FLAGS_cam_id));
  }

  RET_CHECK(capture.isOpened());

  cv::VideoWriter writer;
  const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();
//   if (!save_video) {
  cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
  #if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  capture.set(cv::CAP_PROP_FPS, 30);
#endif
//   }

  LOG(INFO) << "Start running the calculator graph.";
  
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller poller,
                   graph->AddOutputStreamPoller(kOutputStream, true));
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller handedness_poller,
                   graph->AddOutputStreamPoller(kHandednessStream, true));
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller hand_landmarks_poller,
                   graph->AddOutputStreamPoller(kHandLandmarksStream, true));
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller scaled_landmarks_poller,
                   graph->AddOutputStreamPoller(kScaledLandmarksStream, true));
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller hand_rect_poller,
                   graph->AddOutputStreamPoller(kHandRectsStream, true));
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller hand_gestures_poller,
                   graph->AddOutputStreamPoller(kHandGesturesStream, true));
  
  MP_RETURN_IF_ERROR(graph->StartRun({}));
  
  bool grab_frames = true;
    
  // ZeroMQ setup
  const std::string& address = "tcp://127.0.0.1:7000";
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);
  
  LOG(INFO) << "Sending the data through ZMQ via " << address << std::endl;
  socket.connect(address);
    
  int frame_num = 0;
  zmq::message_t reply;
    
//   std::vector<std::string> hand_gestures;
  
  LOG(INFO) << "Start grabbing and processing frames.";
  while (grab_frames) {
    // Capture opencv camera or video frame.
    cv::Mat camera_frame_raw;
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) break;  // End of video.
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);

    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<::mediapipe::ImageFrame>(
        ::mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        ::mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = ::mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Send image packet into the graph.
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph->AddPacketToInputStream(
        kInputStream, ::mediapipe::Adopt(input_frame.release())
                          .At(::mediapipe::Timestamp(frame_timestamp_us)))); 

    // Get the graph result packet, or stop if that fails.
    ::mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    auto& output_frame = packet.Get<::mediapipe::ImageFrame>();
      
    // Excplicitly allow only 2 hands on the frame at most
    std::vector<std::string> global_handedness = {"unkwown", "uknown"};
      
    // This stream is needed to pass info to ZeroMQ
    std::stringstream ss;
    ss << frame_num << std::endl;
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    ss << ms << std::endl;

    // Get the packet containing type of each hand in a corresponding order
    ::mediapipe::Packet handedness_packet;
    if (!handedness_poller.Next(&handedness_packet)) break;
    if (!handedness_packet.IsEmpty()) {
      const auto& handedness = handedness_packet.Get<std::vector<::mediapipe::ClassificationList>>();
      LOG(INFO) << handedness.size() << std::endl;
      for (int i = 0; i < std::min(handedness.size(), global_handedness.size()); ++i) {
        global_handedness[i] = handedness[i].classification(0).label();
        LOG(INFO) << handedness[i].classification(0).label() << std::endl; 
      }
    }
    
    // Get the packets containing hand_landmarks.
    ::mediapipe::Packet hand_landmarks_packet;
    if (!hand_landmarks_poller.Next(&hand_landmarks_packet)) break;
    if (!hand_landmarks_packet.IsEmpty()) {
      const auto& hand_landmarks = hand_landmarks_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
      for (const auto& x: hand_landmarks) {
        PrintLandmarks(x, ss, 640, 480, 1);
//         PrintLandmarks(x, LOG(INFO), 640, 480, 1);
      }
    }

    // Get the packets containing scaled_landmarks.
    ::mediapipe::Packet scaled_landmarks_packet;
    if (!scaled_landmarks_poller.Next(&scaled_landmarks_packet)) break;
    if (!scaled_landmarks_packet.IsEmpty()) {
      const auto& scaled_landmarks = scaled_landmarks_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
      for (const auto& x: scaled_landmarks) {
        PrintLandmarks(x, ss, 1, 1, 1);
//         PrintLandmarks(x, LOG(INFO), 1, 1, 1);
      }
    }
    
    // Get the packets containing hand_gestures.
    ::mediapipe::Packet hand_gestures_packet;
    if (!hand_gestures_poller.Next(&hand_gestures_packet)) break;
    if (!hand_gestures_packet.IsEmpty()) {
      const auto& hand_gestures = hand_gestures_packet.Get<std::vector<std::string>>();
//       hand_gestures = hand_gestures_packet.Get<std::vector<std::string>>();
      for (int i = 0; i < std::min(hand_gestures.size(), global_handedness.size()); ++i) {
          LOG(INFO) << global_handedness[i] << " hand_gesture: " << hand_gestures[i] << std::endl;
          // Currently the Video-Touch system works with the right hand only
          if (global_handedness[i] == "Right") {
              ss << hand_gestures[i] << std::endl;
          }
      }
    }
      
    // Get the packets containing hand_rects.
    ::mediapipe::Packet hand_rects_packet;
    if (!hand_rect_poller.Next(&hand_rects_packet)) break;
    if (!hand_rects_packet.IsEmpty()) {
      const auto& hand_rects = hand_rects_packet.Get<std::vector<::mediapipe::NormalizedRect>>();
      for (int i = 0; i < std::min(hand_rects.size(), global_handedness.size()); ++i) {
          float square = hand_rects[i].height() * hand_rects[i].width();
          LOG(INFO) << global_handedness[i] << " hand_rect square: " << square << std::endl;
          // Currently the Video-Touch system works with the right hand only
          if (global_handedness[i] == "Right") {
              ss << "rect square: " << square << std::endl;
          }
      }
    }

    LOG(INFO) << std::endl;
    
    // Send info to ZeroMQ
    const std::string& info_str = ss.str();
    zmq::message_t msg(info_str.size());
    std::memcpy(msg.data(), info_str.data(), info_str.size());
    socket.send(msg);
    
    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = ::mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    if (save_video) {
      if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(absl::GetFlag(FLAGS_output_video_path),
                    ::mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
      }
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;
    }
      
    // Wait for a servers' reply
    socket.recv(&reply);
    std::string reply_str = std::string(static_cast<char*>(reply.data()), reply.size());
      
    frame_num += 1;
  }
    
  // Finish the communication
  socket.send(zmq::str_buffer("EOQ"));
    
  // Close all resources
  socket.close();

  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph->CloseInputStream(kInputStream));
  return graph->WaitUntilDone();
}

::mediapipe::Status InitializeAndRunMPPGraph() {

  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(::mediapipe::file::GetContents(
      kCalculatorGraphConfigFile, &calculator_graph_config_contents));
  LOG(INFO) << "Get calculator graph config contents: "
            << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  LOG(INFO) << "Initialize the calculator graph.";
  std::unique_ptr<::mediapipe::CalculatorGraph> graph =
      absl::make_unique<::mediapipe::CalculatorGraph>();
  MP_RETURN_IF_ERROR(graph->Initialize(config));

  return RunMPPGraph(std::move(graph));
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  absl::Status run_status = InitializeAndRunMPPGraph();
  if (!run_status.ok()) {
    LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    return EXIT_FAILURE;
  } else {
    LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}
