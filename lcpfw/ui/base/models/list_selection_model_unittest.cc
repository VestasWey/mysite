// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/models/list_selection_model.h"

#include <stddef.h>

#include <algorithm>
#include <string>

#include "base/strings/string_number_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ui {

typedef testing::Test ListSelectionModelTest;

// Returns the state of the selection model as a string. The format is:
// 'active=X anchor=X selection=X X X...'.
static std::string StateAsString(const ListSelectionModel& model) {
  std::string result = "active=" + base::NumberToString(model.active()) +
                       " anchor=" + base::NumberToString(model.anchor()) +
                       " selection=";
  const ListSelectionModel::SelectedIndices& selection(
      model.selected_indices());
  bool first = true;
  for (int index : selection) {
    if (first) {
      first = false;
    } else {
      result += " ";
    }
    result += base::NumberToString(index);
  }
  return result;
}

TEST_F(ListSelectionModelTest, InitialState) {
  ListSelectionModel model;
  EXPECT_EQ("active=-1 anchor=-1 selection=", StateAsString(model));
  EXPECT_TRUE(model.empty());
}

TEST_F(ListSelectionModelTest, SetSelectedIndex) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);
  EXPECT_EQ("active=2 anchor=2 selection=2", StateAsString(model));
  EXPECT_FALSE(model.empty());
}

TEST_F(ListSelectionModelTest, SetSelectedIndexToEmpty) {
  ListSelectionModel model;
  model.SetSelectedIndex(-1);
  EXPECT_EQ("active=-1 anchor=-1 selection=", StateAsString(model));
  EXPECT_TRUE(model.empty());
}

TEST_F(ListSelectionModelTest, IncrementFrom) {
  ListSelectionModel model;
  model.SetSelectedIndex(1);
  model.IncrementFrom(1);
  EXPECT_EQ("active=2 anchor=2 selection=2", StateAsString(model));

  // Increment from 4. This shouldn't effect the selection as its past the
  // end of the selection.
  model.IncrementFrom(4);
  EXPECT_EQ("active=2 anchor=2 selection=2", StateAsString(model));
}

TEST_F(ListSelectionModelTest, DecrementFrom) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);
  model.DecrementFrom(0);
  EXPECT_EQ("active=1 anchor=1 selection=1", StateAsString(model));

  // Shift down from 1. As the selection as the index being removed, this should
  // clear the selection.
  model.DecrementFrom(1);
  EXPECT_EQ("active=-1 anchor=-1 selection=", StateAsString(model));

  // Reset the selection to 2, and shift down from 4. This shouldn't do
  // anything.
  model.SetSelectedIndex(2);
  model.DecrementFrom(4);
  EXPECT_EQ("active=2 anchor=2 selection=2", StateAsString(model));
}

TEST_F(ListSelectionModelTest, IsSelected) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);
  EXPECT_FALSE(model.IsSelected(0));
  EXPECT_TRUE(model.IsSelected(2));
}

TEST_F(ListSelectionModelTest, AddIndexToSelected) {
  ListSelectionModel model;
  model.AddIndexToSelection(2);
  EXPECT_EQ("active=-1 anchor=-1 selection=2", StateAsString(model));

  model.AddIndexToSelection(4);
  EXPECT_EQ("active=-1 anchor=-1 selection=2 4", StateAsString(model));
}

TEST_F(ListSelectionModelTest, AddIndexRangeToSelection) {
  ListSelectionModel model;
  model.AddIndexRangeToSelection(2, 3);
  EXPECT_EQ("active=-1 anchor=-1 selection=2 3", StateAsString(model));

  model.AddIndexRangeToSelection(4, 5);
  EXPECT_EQ("active=-1 anchor=-1 selection=2 3 4 5", StateAsString(model));

  model.AddIndexRangeToSelection(1, 1);
  EXPECT_EQ("active=-1 anchor=-1 selection=1 2 3 4 5", StateAsString(model));
}

TEST_F(ListSelectionModelTest, RemoveIndexFromSelection) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);
  model.AddIndexToSelection(4);
  EXPECT_EQ("active=2 anchor=2 selection=2 4", StateAsString(model));

  model.RemoveIndexFromSelection(4);
  EXPECT_EQ("active=2 anchor=2 selection=2", StateAsString(model));

  model.RemoveIndexFromSelection(2);
  EXPECT_EQ("active=2 anchor=2 selection=", StateAsString(model));
}

TEST_F(ListSelectionModelTest, SetSelectionFromAnchorTo) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);
  model.SetSelectionFromAnchorTo(7);
  EXPECT_EQ("active=7 anchor=2 selection=2 3 4 5 6 7", StateAsString(model));

  model.Clear();
  model.SetSelectedIndex(7);
  model.SetSelectionFromAnchorTo(2);
  EXPECT_EQ("active=2 anchor=7 selection=2 3 4 5 6 7", StateAsString(model));

  model.Clear();
  model.SetSelectionFromAnchorTo(7);
  EXPECT_EQ("active=7 anchor=7 selection=7", StateAsString(model));
}

TEST_F(ListSelectionModelTest, Clear) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);

  model.Clear();
  EXPECT_EQ("active=-1 anchor=-1 selection=", StateAsString(model));
}

TEST_F(ListSelectionModelTest, MoveToLeft) {
  ListSelectionModel model;
  model.SetSelectedIndex(0);
  model.AddIndexToSelection(4);
  model.AddIndexToSelection(10);
  model.set_anchor(4);
  model.set_active(4);
  EXPECT_EQ("active=4 anchor=4 selection=0 4 10", StateAsString(model));
  model.Move(4, 0, 1);
  EXPECT_EQ("active=0 anchor=0 selection=0 1 10", StateAsString(model));
  model.Move(25, 1, 5);
  EXPECT_EQ("active=0 anchor=0 selection=0 6 15", StateAsString(model));
  model.Move(5, 1, 2);
  EXPECT_EQ("active=0 anchor=0 selection=0 2 15", StateAsString(model));
  model.Move(2, 0, 4);
  EXPECT_EQ("active=4 anchor=4 selection=0 4 15", StateAsString(model));
  model.Move(1, 2, 1);
  EXPECT_EQ("active=4 anchor=4 selection=0 4 15", StateAsString(model));
  model.Move(100, 5, 100000);
  EXPECT_EQ("active=4 anchor=4 selection=0 4 100015", StateAsString(model));
  model.Move(4, 0, 200000);
  EXPECT_EQ("active=0 anchor=0 selection=0 100011 200000",
            StateAsString(model));
  model.Move(100011, 1, 1);
  EXPECT_EQ("active=0 anchor=0 selection=0 1 200000", StateAsString(model));
  model.Move(200000, 1, 1);
  EXPECT_EQ("active=0 anchor=0 selection=0 1 2", StateAsString(model));
  model.AddIndexToSelection(4);
  model.AddIndexToSelection(3);
  EXPECT_EQ("active=0 anchor=0 selection=0 1 2 3 4", StateAsString(model));
  model.Move(3, 0, 3);
  EXPECT_EQ("active=3 anchor=3 selection=0 1 3 4 5", StateAsString(model));
  model.Move(3, 1, 10);
  EXPECT_EQ("active=1 anchor=1 selection=0 1 2 3 11", StateAsString(model));
}

TEST_F(ListSelectionModelTest, MoveToRight) {
  ListSelectionModel model;
  model.SetSelectedIndex(0);
  model.AddIndexToSelection(4);
  model.AddIndexToSelection(10);
  model.set_anchor(0);
  model.set_active(0);
  EXPECT_EQ("active=0 anchor=0 selection=0 4 10", StateAsString(model));
  model.Move(0, 3, 1);
  EXPECT_EQ("active=3 anchor=3 selection=3 4 10", StateAsString(model));
  model.Move(2, 4, 4);
  EXPECT_EQ("active=5 anchor=5 selection=5 6 10", StateAsString(model));
  model.Move(5, 6, 1);
  EXPECT_EQ("active=6 anchor=6 selection=5 6 10", StateAsString(model));
  model.Move(5, 6, 2);
  EXPECT_EQ("active=7 anchor=7 selection=6 7 10", StateAsString(model));
  model.Move(1, 2, 3);
  EXPECT_EQ("active=7 anchor=7 selection=6 7 10", StateAsString(model));
  model.Move(1, 6, 4);
  EXPECT_EQ("active=3 anchor=3 selection=2 3 10", StateAsString(model));
  model.Move(0, 7000000, 3);
  EXPECT_EQ("active=0 anchor=0 selection=0 7 7000002", StateAsString(model));
  model.Move(10, 30, 7000000);
  EXPECT_EQ("active=0 anchor=0 selection=0 7 7000022", StateAsString(model));
  model.AddIndexToSelection(10);
  model.AddIndexToSelection(20);
  model.AddIndexToSelection(21);
  EXPECT_EQ("active=0 anchor=0 selection=0 7 10 20 21 7000022",
            StateAsString(model));
  model.Move(22, 9000000, 7000000);
  EXPECT_EQ("active=0 anchor=0 selection=0 7 10 20 21 22",
            StateAsString(model));
  model.Move(0, 10, 10);
  EXPECT_EQ("active=10 anchor=10 selection=0 10 17 20 21 22",
            StateAsString(model));
  model.Move(1, 10, 10);
  EXPECT_EQ("active=19 anchor=19 selection=0 7 19 20 21 22",
            StateAsString(model));
}

TEST_F(ListSelectionModelTest, Copy) {
  ListSelectionModel model;
  model.SetSelectedIndex(0);
  model.AddIndexToSelection(4);
  model.AddIndexToSelection(10);
  EXPECT_EQ("active=0 anchor=0 selection=0 4 10", StateAsString(model));
  ListSelectionModel model2;
  model2 = model;
  EXPECT_EQ("active=0 anchor=0 selection=0 4 10", StateAsString(model2));
}

TEST_F(ListSelectionModelTest, AddSelectionFromAnchorTo) {
  ListSelectionModel model;
  model.SetSelectedIndex(2);

  model.AddSelectionFromAnchorTo(4);
  EXPECT_EQ("active=4 anchor=2 selection=2 3 4", StateAsString(model));

  model.AddSelectionFromAnchorTo(0);
  EXPECT_EQ("active=0 anchor=2 selection=0 1 2 3 4", StateAsString(model));
}

TEST_F(ListSelectionModelTest, Equals) {
  ListSelectionModel model1;
  model1.SetSelectedIndex(0);
  model1.AddSelectionFromAnchorTo(4);

  ListSelectionModel model2;
  model2.SetSelectedIndex(0);
  model2.AddSelectionFromAnchorTo(4);

  EXPECT_TRUE(model1 == model2);
  EXPECT_TRUE(model2 == model1);

  model2.SetSelectedIndex(0);
  EXPECT_FALSE(model1 == model2);
  EXPECT_FALSE(model2 == model1);
}

}  // namespace ui
