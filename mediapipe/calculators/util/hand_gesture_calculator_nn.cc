#include <cmath>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
// #include "tensorflow/lite/tools/gen_op_registration.h"

namespace mediapipe
{

namespace
{
constexpr char normRectTag[] = "NORM_RECT";
constexpr char scaledLandmarkPresenceTag[] = "PRESENCE";
constexpr char scaledLandmarkListTag[] = "SCALED_LANDMARKS";
constexpr char handGestureTag[] = "HAND_GESTURE";
} // namespace

// Graph config:
//
// node {
//   calculator: "HandGestureRecognitionCalculator2"
//   input_stream: "NORM_LANDMARKS:scaled_landmarks"
//   input_stream: "NORM_RECT:hand_rect_for_next_frame"
// }
class HandGestureCalculatorNN : public CalculatorBase
{
public:
    HandGestureCalculatorNN(): CalculatorBase() {
        const std::string filename = "mediapipe/models/gesture_classifier.tflite";
        model = tflite::FlatBufferModel::BuildFromFile(filename.c_str());
        
        if (!model) {
            printf("Failed to mmap model\n");
            exit(0);
        }
        
        tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter_);
        
        interpreter_->AllocateTensors();
        interpreter_->ResetVariableTensors();
        
        LOG(INFO) << "Interpreter loaded" << std::endl;
        LOG(INFO) << "tensors size: " << interpreter_->tensors_size() << std::endl;
        LOG(INFO) << "nodes size: " << interpreter_->nodes_size() << std::endl;
        LOG(INFO) << "inputs: " << interpreter_->inputs().size() << std::endl;
        LOG(INFO) << "input(0) name: " << interpreter_->GetInputName(0) << std::endl;
        LOG(INFO) << "outputs: " << interpreter_->outputs().size() << std::endl;
        LOG(INFO) << "output(0) name: " << interpreter_->GetOutputName(0) << std::endl;
        
        id_to_gesture_name = {"no_gesture", "move", "angle", "grab"};
    }

    static ::mediapipe::Status GetContract(CalculatorContract *cc);
    ::mediapipe::Status Open(CalculatorContext *cc) override;

    ::mediapipe::Status Process(CalculatorContext *cc) override;

private:
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    tflite::ops::builtin::BuiltinOpResolver resolver;
    
    std::vector<std::string> id_to_gesture_name;
};

REGISTER_CALCULATOR(HandGestureCalculatorNN);

::mediapipe::Status HandGestureCalculatorNN::GetContract(
    CalculatorContract *cc)
{
    RET_CHECK(cc->Inputs().HasTag(scaledLandmarkPresenceTag));
    cc->Inputs().Tag(scaledLandmarkPresenceTag).Set<bool>();
    
    RET_CHECK(cc->Inputs().HasTag(scaledLandmarkListTag));
    cc->Inputs().Tag(scaledLandmarkListTag).Set<mediapipe::NormalizedLandmarkList>();

    RET_CHECK(cc->Inputs().HasTag(normRectTag));
    cc->Inputs().Tag(normRectTag).Set<NormalizedRect>();

    RET_CHECK(cc->Outputs().HasTag(handGestureTag));
    cc->Outputs().Tag(handGestureTag).Set<std::string>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureCalculatorNN::Open(
    CalculatorContext *cc)
{
    cc->SetOffset(TimestampDiff(0)); 
    return ::mediapipe::OkStatus();
}

::mediapipe::Status HandGestureCalculatorNN::Process(
    CalculatorContext *cc)
{
    std::string *hand_gesture;
    
    const auto presence = cc->Inputs().Tag(scaledLandmarkPresenceTag).Get<bool>();
    if (!presence) {
        hand_gesture = new std::string("———");
        cc->Outputs()
            .Tag(handGestureTag)
            .Add(hand_gesture, cc->InputTimestamp());
        return ::mediapipe::OkStatus();
    }

    // hand closed (red) rectangle
    const auto rect = &(cc->Inputs().Tag(normRectTag).Get<NormalizedRect>());
    float width = rect->width();
    float height = rect->height();

    if (width < 0.01 || height < 0.01)
    {
        hand_gesture = new std::string("———");
        cc->Outputs()
            .Tag(handGestureTag)
            .Add(hand_gesture, cc->InputTimestamp());
        return ::mediapipe::OkStatus();
    }

    const auto &landmarkList = cc->Inputs()
                                   .Tag(scaledLandmarkListTag)
                                   .Get<mediapipe::NormalizedLandmarkList>();
    RET_CHECK_GT(landmarkList.landmark_size(), 0) << "Input landmark vector is empty.";
    
    int input = interpreter_->inputs()[0];
    float* input_data_ptr = interpreter_->typed_input_tensor<float>(input);
    
    for (int i = 0; i < landmarkList.landmark_size(); ++i) {
        *(input_data_ptr) = (float)landmarkList.landmark(i).x();
        input_data_ptr++;
        *(input_data_ptr) = (float)landmarkList.landmark(i).y();
        input_data_ptr++;
        *(input_data_ptr) = (float)landmarkList.landmark(i).z();
        input_data_ptr++;
    }
    
    interpreter_->Invoke();
    
    float* output = interpreter_->typed_output_tensor<float>(0);
    
    std::vector<float> scores(output, output + 4);
    
    std::vector<float>::iterator max = max_element(scores.begin(), scores.end());
    int argmaxVal = distance(scores.begin(), max);
    
    hand_gesture = new std::string(id_to_gesture_name[argmaxVal]);

    cc->Outputs()
        .Tag(handGestureTag)
        .Add(hand_gesture, cc->InputTimestamp());

    return ::mediapipe::OkStatus();
} // namespace mediapipe

} // namespace mediapipe
