#include "displayapp/screens/Snake.h"
#include <cstdlib>

using namespace Pinetime::Applications::Screens;

Snake::Snake() {
  scoreLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(scoreLabel, "0");
  lv_obj_align(scoreLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  gameOverLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(gameOverLabel, "Game Over\nTap to restart");
  lv_label_set_align(gameOverLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(gameOverLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(gameOverLabel, true);

  Reset();

  taskRefresh = lv_task_create(RefreshTaskCallback, 150, LV_TASK_PRIO_MID, this);
}

Snake::~Snake() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

lv_obj_t* Snake::CreateSegment(int x, int y) {
  lv_obj_t* obj = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_radius(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(obj, cellSize - 1, cellSize - 1);
  lv_obj_set_pos(obj, x * cellSize, y * cellSize);
  return obj;
}

void Snake::Reset() {
  for (auto& obj : bodyObjects) {
    if (obj != nullptr) {
      lv_obj_del(obj);
      obj = nullptr;
    }
  }
  if (foodObject != nullptr) {
    lv_obj_del(foodObject);
    foodObject = nullptr;
  }

  length = 3;
  headIndex = 2;
  body[0] = {4, 6};
  body[1] = {5, 6};
  body[2] = {6, 6};
  bodyObjects[0] = CreateSegment(4, 6);
  bodyObjects[1] = CreateSegment(5, 6);
  bodyObjects[2] = CreateSegment(6, 6);

  direction = Direction::Right;
  pendingDirection = Direction::Right;
  score = 0;
  gameOver = false;
  lv_label_set_text_fmt(scoreLabel, "%lu", static_cast<unsigned long>(score));
  lv_obj_set_hidden(gameOverLabel, true);

  SpawnFood();
}

void Snake::SpawnFood() {
  int x = 0;
  int y = 0;
  bool occupied;
  do {
    x = rand() % nCols;
    y = rand() % nRows;
    occupied = false;
    for (size_t i = 0; i < length; i++) {
      size_t idx = (headIndex + maxLength - i) % maxLength;
      if (body[idx].x == x && body[idx].y == y) {
        occupied = true;
        break;
      }
    }
  } while (occupied);

  food = {static_cast<int8_t>(x), static_cast<int8_t>(y)};
  if (foodObject == nullptr) {
    foodObject = CreateSegment(x, y);
    lv_obj_set_style_local_bg_color(foodObject, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else {
    lv_obj_set_pos(foodObject, x * cellSize, y * cellSize);
  }
}

void Snake::EndGame() {
  gameOver = true;
  lv_obj_set_hidden(gameOverLabel, false);
}

void Snake::Refresh() {
  if (gameOver) {
    return;
  }

  direction = pendingDirection;

  Cell newHead = body[headIndex];
  switch (direction) {
    case Direction::Up:
      newHead.y--;
      break;
    case Direction::Down:
      newHead.y++;
      break;
    case Direction::Left:
      newHead.x--;
      break;
    case Direction::Right:
      newHead.x++;
      break;
  }

  if (newHead.x < 0 || newHead.x >= nCols || newHead.y < 0 || newHead.y >= nRows) {
    EndGame();
    return;
  }

  for (size_t i = 0; i < length; i++) {
    size_t idx = (headIndex + maxLength - i) % maxLength;
    if (body[idx].x == newHead.x && body[idx].y == newHead.y) {
      EndGame();
      return;
    }
  }

  bool ateFood = (newHead.x == food.x && newHead.y == food.y);
  size_t tailIndex = (headIndex + maxLength - (length - 1)) % maxLength;

  headIndex = (headIndex + 1) % maxLength;
  body[headIndex] = newHead;
  bodyObjects[headIndex] = CreateSegment(newHead.x, newHead.y);

  if (ateFood) {
    length++;
    score++;
    lv_label_set_text_fmt(scoreLabel, "%lu", static_cast<unsigned long>(score));
    SpawnFood();
  } else {
    lv_obj_del(bodyObjects[tailIndex]);
    bodyObjects[tailIndex] = nullptr;
  }
}

bool Snake::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (gameOver) {
    if (event == TouchEvents::Tap) {
      Reset();
    }
    return true;
  }

  switch (event) {
    case TouchEvents::SwipeUp:
      if (direction != Direction::Down) {
        pendingDirection = Direction::Up;
      }
      return true;
    case TouchEvents::SwipeDown:
      if (direction != Direction::Up) {
        pendingDirection = Direction::Down;
      }
      return true;
    case TouchEvents::SwipeLeft:
      if (direction != Direction::Right) {
        pendingDirection = Direction::Left;
      }
      return true;
    case TouchEvents::SwipeRight:
      if (direction != Direction::Left) {
        pendingDirection = Direction::Right;
      }
      return true;
    default:
      return false;
  }
}
