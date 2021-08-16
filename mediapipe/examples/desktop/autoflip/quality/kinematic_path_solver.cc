#include "mediapipe/examples/desktop/autoflip/quality/kinematic_path_solver.h"

namespace mediapipe {
namespace autoflip {
namespace {
int Median(const std::deque<std::pair<uint64, int>>& positions_raw) {
  std::deque<int> positions;
  for (const auto& position : positions_raw) {
    positions.push_back(position.second);
  }

  size_t n = positions.size() / 2;
  nth_element(positions.begin(), positions.begin() + n, positions.end());
  return positions[n];
}
}  // namespace
bool KinematicPathSolver::IsMotionTooSmall(double delta_degs) {
  if (options_.has_min_motion_to_reframe()) {
    return abs(delta_degs) < options_.min_motion_to_reframe();
  } else if (delta_degs > 0) {
    return delta_degs < options_.min_motion_to_reframe_upper();
  } else {
    return abs(delta_degs) < options_.min_motion_to_reframe_lower();
  }
}
void KinematicPathSolver::ClearHistory() { raw_positions_at_time_.clear(); }
absl::Status KinematicPathSolver::PredictMotionState(int position,
                                                     const uint64 time_us,
                                                     bool* state) {
  if (!initialized_) {
    *state = false;
    return absl::OkStatus();
  }

  auto raw_positions_at_time_copy = raw_positions_at_time_;

  raw_positions_at_time_copy.push_front(
      std::pair<uint64, int>(time_us, position));
  while (raw_positions_at_time_copy.size() > 1) {
    if (static_cast<int64>(raw_positions_at_time_copy.back().first) <
        static_cast<int64>(time_us) - options_.filtering_time_window_us()) {
      raw_positions_at_time_copy.pop_back();
    } else {
      break;
    }
  }

  int filtered_position = Median(raw_positions_at_time_copy);
  double delta_degs =
      (filtered_position - current_position_px_) / pixels_per_degree_;

  // If the motion is smaller than the min_motion_to_reframe and camera is
  // stationary, don't use the update.
  if (IsMotionTooSmall(delta_degs) && !motion_state_) {
    *state = false;
  } else if (abs(delta_degs) < options_.reframe_window() && motion_state_) {
    // If the motion is smaller than the reframe_window and camera is moving,
    // don't use the update.
    *state = false;
  } else {
    // Apply new position, plus the reframe window size.
    *state = true;
  }

  return absl::OkStatus();
}
absl::Status KinematicPathSolver::AddObservation(int position,
                                                 const uint64 time_us) {
  if (!initialized_) {
    if (position < min_location_) {
      current_position_px_ = min_location_;
    } else if (position > max_location_) {
      current_position_px_ = max_location_;
    } else {
      current_position_px_ = position;
    }
    target_position_px_ = position;
    motion_state_ = false;
    mean_delta_t_ = -1;
    raw_positions_at_time_.push_front(
        std::pair<uint64, int>(time_us, position));
    current_time_ = time_us;
    initialized_ = true;
    current_velocity_deg_per_s_ = 0;
    RET_CHECK_GT(pixels_per_degree_, 0)
        << "pixels_per_degree must be larger than 0.";
    RET_CHECK_GE(options_.update_rate_seconds(), 0)
        << "update_rate_seconds must be greater than 0.";
    RET_CHECK_GE(options_.filtering_time_window_us(), 0)
        << "update_rate_seconds must be greater than 0.";
    RET_CHECK_GE(options_.mean_period_update_rate(), 0)
        << "mean_period_update_rate must be greater than 0.";
    RET_CHECK(options_.has_min_motion_to_reframe() ^
              (options_.has_min_motion_to_reframe_upper() &&
               options_.has_min_motion_to_reframe_lower()))
        << "Must set min_motion_to_reframe or min_motion_to_reframe_upper and "
           "min_motion_to_reframe_lower.";
    if (options_.has_min_motion_to_reframe()) {
      RET_CHECK_GE(options_.min_motion_to_reframe(), options_.reframe_window())
          << "Reframe window cannot exceed min_motion_to_reframe.";
    } else {
      RET_CHECK_GE(options_.min_motion_to_reframe_upper(),
                   options_.reframe_window())
          << "Reframe window cannot exceed min_motion_to_reframe.";
      RET_CHECK_GE(options_.min_motion_to_reframe_lower(),
                   options_.reframe_window())
          << "Reframe window cannot exceed min_motion_to_reframe.";
    }
    return absl::OkStatus();
  }

  RET_CHECK(current_time_ < time_us)
      << "Observation added before a prior observations.";

  raw_positions_at_time_.push_front(std::pair<uint64, int>(time_us, position));
  while (raw_positions_at_time_.size() > 1) {
    if (static_cast<int64>(raw_positions_at_time_.back().first) <
        static_cast<int64>(time_us) - options_.filtering_time_window_us()) {
      raw_positions_at_time_.pop_back();
    } else {
      break;
    }
  }

  int filtered_position = Median(raw_positions_at_time_);
  double delta_degs =
      (filtered_position - current_position_px_) / pixels_per_degree_;

  // If the motion is smaller than the min_motion_to_reframe and camera is
  // stationary, don't use the update.
  if (IsMotionTooSmall(delta_degs) && !motion_state_) {
    delta_degs = 0;
    motion_state_ = false;
  } else if (abs(delta_degs) < options_.reframe_window() && motion_state_) {
    // If the motion is smaller than the reframe_window and camera is moving,
    // don't use the update.
    delta_degs = 0;
    motion_state_ = false;
  } else if (delta_degs > 0) {
    // Apply new position, less the reframe window size.
    target_position_px_ =
        filtered_position - pixels_per_degree_ * options_.reframe_window();
    delta_degs =
        (target_position_px_ - current_position_px_) / pixels_per_degree_;
    motion_state_ = true;
  } else {
    // Apply new position, plus the reframe window size.
    target_position_px_ =
        filtered_position + pixels_per_degree_ * options_.reframe_window();
    delta_degs =
        (target_position_px_ - current_position_px_) / pixels_per_degree_;
    motion_state_ = true;
  }

  // Time and position updates.
  double delta_t = (time_us - current_time_) / 1000000.0;
  // Time since last state/prediction update, smoothed by
  // mean_period_update_rate.
  if (mean_delta_t_ < 0) {
    mean_delta_t_ = delta_t;
  } else {
    mean_delta_t_ = mean_delta_t_ * (1 - options_.mean_period_update_rate()) +
                    delta_t * options_.mean_period_update_rate();
  }

  // Observed velocity and then weighted update of this velocity.
  double observed_velocity = delta_degs / delta_t;
  double update_rate = std::min(mean_delta_t_ / options_.update_rate_seconds(),
                                options_.max_update_rate());
  double updated_velocity = current_velocity_deg_per_s_ * (1 - update_rate) +
                            observed_velocity * update_rate;
  // Limited current velocity.
  current_velocity_deg_per_s_ =
      updated_velocity > 0 ? fmin(updated_velocity, options_.max_velocity())
                           : fmax(updated_velocity, -options_.max_velocity());

  // Update prediction based on time input.
  return UpdatePrediction(time_us);
}

absl::Status KinematicPathSolver::UpdatePrediction(const int64 time_us) {
  RET_CHECK(current_time_ < time_us)
      << "Prediction time added before a prior observation or prediction.";

  // Position update limited by min/max.
  double update_position_px =
      current_position_px_ +
      current_velocity_deg_per_s_ * mean_delta_t_ * pixels_per_degree_;

  if (update_position_px < min_location_) {
    current_position_px_ = min_location_;
    current_velocity_deg_per_s_ = 0;
    motion_state_ = false;
  } else if (update_position_px > max_location_) {
    current_position_px_ = max_location_;
    current_velocity_deg_per_s_ = 0;
    motion_state_ = false;
  } else {
    current_position_px_ = update_position_px;
  }
  current_time_ = time_us;

  return absl::OkStatus();
}

absl::Status KinematicPathSolver::GetState(int* position) {
  RET_CHECK(initialized_) << "GetState called before first observation added.";
  *position = round(current_position_px_);
  return absl::OkStatus();
}

absl::Status KinematicPathSolver::SetState(const int position) {
  RET_CHECK(initialized_) << "SetState called before first observation added.";
  current_position_px_ = position;
  return absl::OkStatus();
}

absl::Status KinematicPathSolver::GetTargetPosition(int* target_position) {
  RET_CHECK(initialized_)
      << "GetTargetPosition called before first observation added.";
  *target_position = round(target_position_px_);
  return absl::OkStatus();
}

absl::Status KinematicPathSolver::UpdatePixelsPerDegree(
    const float pixels_per_degree) {
  RET_CHECK_GT(pixels_per_degree, 0)
      << "pixels_per_degree must be larger than 0.";
  pixels_per_degree_ = pixels_per_degree;
  return absl::OkStatus();
}

absl::Status KinematicPathSolver::UpdateMinMaxLocation(const int min_location,
                                                       const int max_location) {
  RET_CHECK(initialized_)
      << "UpdateMinMaxLocation called before first observation added.";
  double prior_distance = max_location_ - min_location_;
  double updated_distance = max_location - min_location;
  double scale_change = updated_distance / prior_distance;
  current_position_px_ = current_position_px_ * scale_change;
  target_position_px_ = target_position_px_ * scale_change;
  max_location_ = max_location;
  min_location_ = min_location;
  auto original_positions_at_time = raw_positions_at_time_;
  raw_positions_at_time_.clear();
  for (auto position_at_time : original_positions_at_time) {
    position_at_time.second = position_at_time.second * scale_change;
    raw_positions_at_time_.push_front(position_at_time);
  }
  return absl::OkStatus();
}

}  // namespace autoflip
}  // namespace mediapipe
