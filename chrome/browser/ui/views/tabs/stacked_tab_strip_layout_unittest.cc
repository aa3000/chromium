// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tabs/stacked_tab_strip_layout.h"

#include <string>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/views/view.h"
#include "ui/views/view_model.h"

namespace {

struct CommonTestData {
  const int initial_x;
  const int width;
  const int tab_size;
  const int tab_padding;
  const int stacked_offset;
  const int mini_tab_count;
  const int active_index;
  const std::string start_bounds;
  const std::string expected_bounds;
};

}  // namespace

class StackedTabStripLayoutTest : public testing::Test {
 public:
  StackedTabStripLayoutTest() {}

 protected:
  void Reset(StackedTabStripLayout* layout,
             int x,
             int width,
             int mini_tab_count,
             int active_index) {
    layout->Reset(x, width, mini_tab_count, active_index);
  }

  void CreateLayout(const CommonTestData& data) {
    if (!data.start_bounds.empty())
      PrepareChildViewsFromString(data.start_bounds);
    else
      PrepareChildViewsFromString(data.expected_bounds);
    layout_.reset(new StackedTabStripLayout(
                     gfx::Size(data.tab_size, 10), data.tab_padding,
                     data.stacked_offset, 4, &view_model_));
    if (data.start_bounds.empty()) {
      PrepareChildViewsFromString(data.expected_bounds);
      layout_->Reset(data.initial_x, data.width, data.mini_tab_count,
                     data.active_index);
    } else {
      ASSERT_NO_FATAL_FAILURE(SetBoundsFromString(data.start_bounds));
      layout_->Reset(data.initial_x, data.width, data.mini_tab_count,
                     data.active_index);
      ASSERT_NO_FATAL_FAILURE(SetBoundsFromString(data.start_bounds));
    }
  }

  void AddViewToViewModel(int index) {
    views::View* child_view = new views::View;
    view_.AddChildView(child_view);
    view_model_.Add(child_view, index);
  }

  void PrepareChildViewsFromString(const std::string& bounds) {
    std::vector<std::string> positions;
    Tokenize(bounds, " ", &positions);
    PrepareChildViews(static_cast<int>(positions.size()));
  }

  void PrepareChildViews(int count) {
    view_model_.Clear();
    view_.RemoveAllChildViews(true);
    for (int i = 0; i < count; ++i)
      AddViewToViewModel(i);
  }

  void SetBoundsFromString(const std::string& bounds) {
    std::vector<std::string> positions;
    Tokenize(bounds, " ", &positions);
    PrepareChildViews(static_cast<int>(positions.size()));
    for (int i = 0; i < view_model_.view_size(); ++i) {
      int x = 0;
      gfx::Rect bounds(view_model_.ideal_bounds(i));
      ASSERT_TRUE(base::StringToInt(positions[i], &x));
      bounds.set_x(x);
      view_model_.set_ideal_bounds(i, bounds);
    }
  }

  std::string BoundsString() const {
    std::string result;
    for (int i = 0; i < view_model_.view_size(); ++i) {
      if (!result.empty())
        result += " ";
      result += base::IntToString(view_model_.ideal_bounds(i).x());
    }
    return result;
  }

  std::string BoundsString2(int active_index) const {
    std::string result;
    for (int i = 0; i < view_model_.view_size(); ++i) {
      if (!result.empty())
        result += " ";
      if (i == active_index)
        result += "[";
      result += base::IntToString(view_model_.ideal_bounds(i).x());
      if (i == active_index)
        result += "]";
    }
    return result;
  }

  void Validate(int active_index, int max_width) {
    // Make sure none of the tabs are more than 90 apart
    // (tab_size(100) + padding (-10)).
    for (int j = 1; j < view_model_.view_size(); ++j)
      EXPECT_LE(ideal_x(j) - ideal_x(j - 1), max_width - 100);
  }

  int ideal_x(int index) const {
    return view_model_.ideal_bounds(index).x();
  }

  scoped_ptr<StackedTabStripLayout> layout_;
  views::ViewModel view_model_;

 private:
  views::View view_;

  DISALLOW_COPY_AND_ASSIGN(StackedTabStripLayoutTest);
};

// Random data.
TEST_F(StackedTabStripLayoutTest, ValidateInitialLayout) {
  StackedTabStripLayout layout(gfx::Size(100, 10), -10, 2, 4, &view_model_);
  PrepareChildViews(12);

  for (int i = 120; i < 600; ++i) {
    for (int j = 0; j < 12; ++j) {
      Reset(&layout, 0, i, 0, j);
      Validate(j, i);
      if (HasNonfatalFailure())
        return;
    }
  }
}

// Ensure initial layout is correct.
TEST_F(StackedTabStripLayoutTest, InitialLayout) {
  struct CommonTestData test_data[] = {
    { 0, 198, 100, -10, 1, 0, 9, "",
      "0 0 0 0 0 0 1 2 3 4 94 95 96 97 98 98 98 98" },
    { 0, 198, 100, -10, 1, 0, 0, "", "0 90 94 95 96 97 98 98 98" },
    { 0, 300, 100, -10, 1, 0, 0, "",
      "0 90 180 196 197 198 199 200 200 200 200" },
    { 0, 300, 100, -10, 1, 0, 10, "", "0 0 0 0 1 2 3 4 20 110 200" },
    { 0, 300, 100, -10, 1, 0, 1, "", "0 90 180 196 197 198 199 200 200" },
    { 0, 643, 160, -27, 6, 0, 0, "", "0 133 266 399" },
    { 0, 300, 100, -10, 1, 0, 7, "", "0 1 2 3 4 20 110 200" },
    { 0, 300, 100, -10, 1, 0, 6, "", "0 1 2 3 4 20 110 200" },
    { 0, 300, 100, -10, 1, 0, 4, "", "0 1 2 3 4 94 184 199 200" },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i]);
    EXPECT_EQ(test_data[i].expected_bounds, BoundsString()) << " at " << i;
  }
}

// Assertions for dragging from an existing configuration.
TEST_F(StackedTabStripLayoutTest, DragActiveTabExisting) {
  struct TestData {
    struct CommonTestData common_data;
    const int delta;
  } test_data[] = {
    //
    // The following set of tests create 6 tabs, the first two are pinned and
    // the 2nd tab is selected.
    //
    // 1 pixel to the right, should push only mini-tabs and first non-mini-tab.
    { { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
        "1 6 11 101 138 140" }, 1 },
    // Push enough to collapse the 4th tab.
    { { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
        "36 41 46 136 138 140" }, 36 },
    // 1 past collapsing the 4th.
    { { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
        "37 42 47 136 138 140" }, 37 },
    // Collapse the third.
    { { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
        "124 129 134 136 138 140" }, 124 },
    // One past collapsing the third.
    { { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
        "124 129 134 136 138 140" }, 125 },

    //
    // The following set of tests create 6 tabs, the first two are pinned and
    // the 5th is selected.
    //
    // 1 pixel to the right, should expose part of a tab.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140", "0 5 10 90 131 140" },
      1 },
    // Push the tab as far to the right as it'll go.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140", "0 5 10 90 138 140" },
      8 },
    // One past as far to the right as it'll go. Should expose more of the tab
    // before it.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140", "0 5 10 91 138 140" },
      9 },
    // Enough so that the pinned tabs start pulling in.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140", "1 6 11 101 138 140" },
      19 },
    // One more than last.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140", "2 7 12 102 138 140" },
      20 },
    // Enough to collapse the fourth as small it can get.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140",
        "36 41 46 136 138 140" }, 54 },
    // Enough to collapse the third as small it can get.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140",
        "124 129 134 136 138 140" }, 142 },
    // One more than last, shouldn't change anything.
    { { 10, 240, 100, -10, 2, 2, 4, "0 5 10 90 130 140",
        "124 129 134 136 138 140" }, 143 },

    //
    // The following set of tests create 3 tabs with the second selected.
    //
    // Drags in 2, pulling the rightmost tab along.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "2 92 140" }, 2 },
    // Drags the rightmost tab as far to right as possible.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "48 138 140" }, 48 },
    // Drags so much that the left most tabs pulls in.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "135 138 140" }, 135 },
    // Drags so far that no more tabs pull in.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "136 138 140" }, 200 },
    // Drags to the left most position before the right tabs start pulling in.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "0 50 140" }, -40 },
    // Drags 1 beyond the left most position, which should pull in the right
    // tab slightly.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "0 49 139" }, -41 },
    // Drags to the left as far as the tab goes.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "0 2 92" }, -88 },
    // Drags one past as far to the left as the tab goes. Should keep pulling
    // in the rightmost tab.
    { { 0, 240, 100, -10, 2, 0, 1, "0 90 140", "0 2 91" }, -89 },
  };

  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    layout_->DragActiveTab(test_data[i].delta);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Assertions for SizeToFit().
TEST_F(StackedTabStripLayoutTest, SizeToFit) {
  struct CommonTestData test_data[] = {
    // Dragged to the right.
    { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140", "1 6 11 101 138 140"},
    { 10, 240, 100, -10, 2, 2, 1, "0 5 10 100 138 140",
      "124 129 134 136 138 140" },

    // Dragged to the left.
    { 0, 240, 100, -10, 2, 0, 1, "0 50 140", "0 49 139" },

    // Dragged to the left.
    { 0, 240, 100, -10, 2, 0, 1, "0 49 89 140", "0 49 89 139" },
  };

  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i]);
    SetBoundsFromString(test_data[i].expected_bounds);
    layout_->SizeToFit();
    // NOTE: because of the way the code is structured this asserts on
    // |start_bound|, not |expected_bounds|.
    EXPECT_EQ(test_data[i].start_bounds, BoundsString()) << " at " << i;
  }
}

// Assertions for AddTab().
TEST_F(StackedTabStripLayoutTest, AddTab) {
  struct TestData {
    CommonTestData common_data;
    int add_index;
    bool add_active;
    bool add_mini;
  } test_data[] = {
    // Adding a background tab test cases.
    { { 0, 300, 100, -10, 2, 0, 1, "0 90 180 198 200", "0 16 106 196 198 200"},
      3, false, false },
    { { 0, 300, 100, -10, 2, 0, 1, "0 90 180 198 200", "0 2 4 20 110 200"},
      5, false, false },
    { { 0, 300, 100, -10, 2, 0, 1, "0 90 180 198 200", "0 90 180 196 198 200"},
      2, false, false },
    { { 0, 300, 100, -10, 2, 0, 1, "0 90 180 198 200", "0 2 4 94 184 200"},
      0, false, false },

    { { 4, 200, 100, -10, 2, 1, 2, "0 4 10 100", "0 0 8 10 100"},
      1, false, true },
    { { 4, 200, 100, -10, 2, 1, 2, "0 4 10 100", "0 0 8 98 100"},
      1, true, true },
    { { 4, 200, 100, -10, 2, 1, 2, "0 4 10 100", "0 0 8 98 100"},
      0, true, true },
    { { 0, 200, 100, -10, 2, 0, 2, "0 2 10 100", "0 4 94 98 100"},
      0, true, true },

    { { 0, 200, 100, -10, 2, 0, 0, "0 90 92 92 94 96 98 100",
                                   "0 0 0 2 4 6 8 98 100"},
      7, true, false },
    { { 0, 200, 100, -10, 2, 0, 7, "0 2 4 6 8 8 10 100",
                                   "0 0 2 4 6 8 96 98 100"},
      5, true, false },
    { { 0, 200, 100, -10, 2, 0, 7, "0 2 4 6 8 8 10 100",
                                   "0 2 4 6 8 94 96 98 100"},
      4, true, false },
    { { 0, 200, 100, -10, 2, 0, 2, "0 2 10 100", "0 2 10 98 100"},
      2, true, false },
    { { 0, 200, 100, -10, 2, 0, 2, "0 2 10 100", "0 2 4 10 100"},
      4, true, false },
    { { 0, 200, 100, -10, 2, 0, 2, "0 2 10 100", "0 90 96 98 100"},
      0, true, false },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    int add_types = 0;
    if (test_data[i].add_active)
      add_types |= StackedTabStripLayout::kAddTypeActive;
    if (test_data[i].add_mini)
      add_types |= StackedTabStripLayout::kAddTypeMini;
    AddViewToViewModel(test_data[i].add_index);
    layout_->AddTab(test_data[i].add_index, add_types,
                    test_data[i].common_data.initial_x +
                    (test_data[i].add_mini ? 4 : 0));
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Assertions around removing tabs.
TEST_F(StackedTabStripLayoutTest, RemoveTab) {
  // TODO: add coverage of removing mini tabs!
  struct TestData {
    struct CommonTestData common_data;
    const int remove_index;
    const int x_after_remove;
  } test_data[] = {
    { { 0, 882, 220, -29, 2, 0, 4, "0 23 214 405 596 602",
        "0 191 382 573 662" }, 1, 0 },

    // Remove before active.
    { { 0, 200, 100, -10, 2, 0, 4, "0 2 4 6 8 10 80 98 100",
        "0 2 6 8 10 80 98 100" },
      2, 0 },

    // Stacked tabs on both sides.
    { { 0, 200, 100, -10, 2, 0, 4, "0 2 4 6 8 10 80 98 100",
        "0 2 4 6 10 80 98 100" },
      4, 0 },

    // Mini-tabs.
    { { 8, 200, 100, -10, 2, 1, 0, "0 8 94 96 98 100", "0 86 88 90 100" },
      0, 0 },
    { { 16, 200, 100, -10, 2, 2, 0, "0 8 16 94 96 98 100", "8 8 86 88 90 100" },
      0, 8 },
    { { 16, 200, 100, -10, 2, 2, 0, "0 8 16 94 96 98 100", "0 8 86 88 90 100" },
      1, 8 },

    // Remove from ideal layout.
    { { 0, 200, 100, -10, 2, 0, 0, "0 90 94 96 98 100", "0 90 96 98 100" },
      0, 0 },
    { { 0, 200, 100, -10, 2, 0, 0, "0 90 94 96 98 100", "0 90 96 98 100" },
      1, 0 },
    { { 0, 200, 100, -10, 2, 0, 0, "0 90 94 96 98 100", "0 90 96 98 100" },
      2, 0 },
    { { 0, 200, 100, -10, 2, 0, 0, "0 90 94 96 98 100", "0 90 94 98 100" },
      3, 0 },
    { { 0, 200, 100, -10, 2, 0, 0, "0 90 94 96 98 100", "0 90 94 96 100" },
      5, 0 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    int old_x = view_model_.ideal_bounds(test_data[i].remove_index).x();
    view_model_.Remove(test_data[i].remove_index);
    layout_->RemoveTab(test_data[i].remove_index, test_data[i].x_after_remove,
                       old_x);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Assertions for SetWidth().
TEST_F(StackedTabStripLayoutTest, SetWidth) {
  struct TestData {
    CommonTestData common_data;
    int new_width;
  } test_data[] = {
    { { 0, 500, 100, -10, 2, 0, 4, "0 90 180 270 360 400",
                                   "0 90 180 196 198 200"}, 300 },

    // Verifies a bug in AdjustTrailingStackedTabs().
    { { 0, 103, 100, -10, 2, 0, 0, "", "0 2"}, 102 },

    { { 8, 250, 100, -10, 2, 2, 2, "0 4 8 98 148 150", "0 4 8 98 160 250"},
      350 },
    { { 8, 250, 100, -10, 2, 2, 2, "0 4 8 98 148 150", "0 4 8 96 98 100"},
      200 },

    { { 0, 250, 100, -10, 2, 0, 2, "0 40 90 120 150", "0 40 90 98 100"}, 200 },
    { { 0, 250, 100, -10, 2, 0, 2, "0 2 60 150", "0 2 60 100"}, 200 },
    { { 0, 250, 100, -10, 2, 0, 2, "0 40 120 150", "0 40 98 100"}, 200 },

    { { 0, 200, 100, -10, 2, 0, 2, "0 2 10 100", "0 2 60 150"}, 250 },
    { { 0, 200, 100, -10, 2, 0, 2, "0 2 4 10 100", "0 2 20 110 200"}, 300 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    layout_->SetWidth(test_data[i].new_width);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Assertions for SetActiveIndex().
TEST_F(StackedTabStripLayoutTest, SetActiveIndex) {
  struct TestData {
    CommonTestData common_data;
    int new_index;
  } test_data[] = {
    { { 0, 250, 100, -10, 2, 0, 2, "0 4 8 98 148 150", "0 90 144 146 148 150"},
      0 },
    { { 0, 250, 100, -10, 2, 0, 2, "0 4 8 98 148 150", "0 2 4 58 148 150"}, 4 },
    { { 0, 250, 100, -10, 2, 0, 2, "0 4 8 98 148 150", "0 2 4 6 60 150"}, 5 },
    { { 4, 250, 100, -10, 2, 1, 2, "0 4 8 98 148 150", "0 4 94 146 148 150"},
      0 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    layout_->SetActiveIndex(test_data[i].new_index);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Makes sure don't crash when resized and only one tab.
TEST_F(StackedTabStripLayoutTest, EmptyTest) {
  StackedTabStripLayout layout(gfx::Size(160, 10), -27, 6, 4, &view_model_);
  PrepareChildViews(1);
  layout.AddTab(0, StackedTabStripLayout::kAddTypeActive, 0);
  layout.SetWidth(100);
  layout.SetWidth(50);
  layout.SetWidth(0);
  layout.SetWidth(500);
}

// Assertions around removing tabs.
TEST_F(StackedTabStripLayoutTest, MoveTab) {
  // TODO: add coverage of removing mini tabs!
  struct TestData {
    struct CommonTestData common_data;
    const int from;
    const int to;
    const int new_active_index;
    const int new_start_x;
    const int new_mini_tab_count;
  } test_data[] = {
    // Moves and unpins.
    { { 10, 300, 100, -10, 2, 2, 0, "", "0 5 10 100 190 198 200" },
      0, 1, 2, 5, 1 },

    // Moves and pins.
    { { 0, 300, 100, -10, 2, 0, 4, "", "0 5 95 185 196 198 200" },
      2, 0, 0, 5, 1 },
    { { 0, 300, 100, -10, 2, 1, 2, "", "0 5 10 100 190 198 200" },
      2, 0, 0, 10, 2 },

    { { 0, 200, 100, -10, 2, 0, 4, "0 2 4 6 96 98 100", "0 2 4 6 96 98 100" },
      2, 0, 4, 0, 0 },
    { { 0, 200, 100, -10, 2, 0, 4, "0 2 4 6 96 98 100", "0 2 4 6 8 10 100" },
      0, 6, 6, 0, 0 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    view_model_.MoveViewOnly(test_data[i].from, test_data[i].to);
    for (int j = 0; j < test_data[i].new_mini_tab_count; ++j) {
      gfx::Rect bounds;
      bounds.set_x(j * 5);
      view_model_.set_ideal_bounds(j, bounds);
    }
    layout_->MoveTab(test_data[i].from, test_data[i].to,
                     test_data[i].new_active_index, test_data[i].new_start_x,
                     test_data[i].new_mini_tab_count);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}

// Assertions around IsStacked().
TEST_F(StackedTabStripLayoutTest, IsStacked) {
  // A single tab with enough space should never be stacked.
  PrepareChildViews(1);
  layout_.reset(new StackedTabStripLayout(
                    gfx::Size(100, 10), -10, 2, 4, &view_model_));
  Reset(layout_.get(), 0, 400, 0, 0);
  EXPECT_FALSE(layout_->IsStacked(0));

  // First tab active, remaining tabs stacked.
  PrepareChildViews(8);
  Reset(layout_.get(), 0, 400, 0, 0);
  EXPECT_FALSE(layout_->IsStacked(0));
  EXPECT_TRUE(layout_->IsStacked(7));

  // Last tab active, preceeding tabs stacked.
  layout_->SetActiveIndex(7);
  EXPECT_FALSE(layout_->IsStacked(7));
  EXPECT_TRUE(layout_->IsStacked(0));
}

// Assertions around SetXAndMiniCount.
TEST_F(StackedTabStripLayoutTest, SetXAndMiniCount) {
  // Verifies we don't crash when transitioning to all mini-tabs.
  PrepareChildViews(1);
  layout_.reset(new StackedTabStripLayout(
                    gfx::Size(100, 10), -10, 2, 4, &view_model_));
  Reset(layout_.get(), 0, 400, 0, 0);
  layout_->SetXAndMiniCount(0, 1);
}

// Assertions around SetXAndMiniCount.
TEST_F(StackedTabStripLayoutTest, SetActiveTabLocation) {
  struct TestData {
    struct CommonTestData common_data;
    const int location;
  } test_data[] = {
    // Active tab is the first tab, can't be moved.
    { { 0, 300, 100, -10, 2, 0, 0, "", "0 90 180 194 196 198 200" }, 50 },

    // Active tab is pinned; should result in nothing.
    { { 0, 300, 100, -10, 2, 2, 1, "", "0 0 0 90 180 198 200" }, 199 },

    // Location is too far to the right, ends up being pushed in.
    { { 0, 300, 100, -10, 2, 0, 3, "", "0 14 104 194 196 198 200" }, 199 },

    // Location can be honored.
    { { 0, 300, 100, -10, 2, 0, 3, "", "0 2 4 40 130 198 200" }, 40 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(test_data); ++i) {
    CreateLayout(test_data[i].common_data);
    layout_->SetActiveTabLocation(test_data[i].location);
    EXPECT_EQ(test_data[i].common_data.expected_bounds, BoundsString()) <<
        " at " << i;
  }
}
