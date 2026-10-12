/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "SampleIterator.h"
#include "gtest/gtest.h"

using mozilla::KeyframeMap;
using mozilla::Nothing;
using mozilla::Some;
using KeyframeInfo = KeyframeMap::KeyframeInfo;
using mozilla::media::TimeUnit;

constexpr int64_t BASE = 1000;

TimeUnit point(int64_t t) { return TimeUnit(t, BASE); }

KeyframeMap::Interval interval(int64_t start, int64_t end) {
  return KeyframeMap::Interval(TimeUnit(start, BASE), TimeUnit(end, BASE));
}

TEST(MP4KeyframeMap, Empty)
{
  KeyframeMap m;
  EXPECT_FALSE(m.GetFirst().isSome());
  EXPECT_FALSE(m.Get(point(100)).isSome());
  EXPECT_FALSE(m.GetNearestUpTo(point(100)));
  m.MarkNotKeyframe(interval(200, 230), KeyframeInfo::Codec);
  m.MarkNotKeyframe(interval(400, 410), KeyframeInfo::Container);
  EXPECT_FALSE(m.GetFirst().isSome());
  EXPECT_FALSE(m.HaveChecked(TimeUnit(400, BASE)));
}

TEST(MP4KeyframeMap, Insert)
{
  KeyframeMap m;
  const auto i1 = interval(80, 100);
  KeyframeMap::Entry e1(i1, {0, 5}, KeyframeInfo::Container);
  m.MarkKeyframe(i1, {0, 5}, KeyframeInfo::Container);
  EXPECT_FALSE(m.HaveChecked(i1.mStart));
  EXPECT_EQ(m.Get(i1.mStart), Some(e1));
  m.MarkKeyframe(i1, {0, 5}, KeyframeInfo::Codec);
  EXPECT_TRUE(m.HaveChecked(i1.mStart));
  EXPECT_TRUE(m.HaveChecked(i1.mStart + point(10)));
  EXPECT_FALSE(m.HaveChecked(point(1)));

  const auto i2 = interval(200, 220);
  KeyframeMap::Entry e2(i2, {0, 8}, KeyframeInfo::Container);
  m.MarkKeyframe(i2, {0, 8}, KeyframeInfo::Container);
  EXPECT_EQ(m.GetNearestUpTo(point(10)), Nothing());
  KeyframeMap::Entry e1Confirmed(i1, {0, 5}, KeyframeInfo::Codec);
  EXPECT_EQ(m.GetNearestUpTo(i1.mStart), Some(e1Confirmed));
  EXPECT_EQ(m.GetNearestUpTo(i1.mStart + point(10)), Some(e1Confirmed));

  EXPECT_EQ(m.GetNextAfter(i1.mStart - point(1)), Some(e1Confirmed));
  EXPECT_EQ(m.GetNextAfter(i1.mEnd), Some(e2));

  EXPECT_EQ(m.GetNextAfter(point(120)), Some(e2));
  EXPECT_EQ(m.GetNearestUpTo(point(300)), Some(e2));
  EXPECT_EQ(m.GetNearestUpTo(point(220)), Some(e2));
  m.MarkNotKeyframe(i2, KeyframeInfo::Codec);
  EXPECT_EQ(m.GetNextAfter(point(120)), Nothing());
  EXPECT_EQ(m.GetNearestUpTo(point(300)), Some(e1Confirmed));
}
