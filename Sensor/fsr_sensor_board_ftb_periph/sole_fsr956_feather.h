#pragma once

// FSR 956 pin sole sensor connections

const int COLUMNS = 21;
const int MuxDC_PIN = 15;
const int MuxADC_PIN = 16;
const int ADC_PINS[ ] = {A0, A1, A2, A3, A6};
const int DC_PINS[ ] = {7, 11, 18, 22, 27, 28, 29};

struct line {
  int adc_pin;
  int dc_pin [COLUMNS];
};

struct line sole[] = {
  {  0, { -1, -1, -1, -1, -1, -1, -1,  4,  5,  6,  7,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1}}, // 1
  {  1, { -1, -1, -1, -1, -1, -1,  3,  4,  5,  6,  7,  8,  9, 10, 11, -1, -1, -1, -1, -1, -1}}, // 2
  {  2, { -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, -1, -1, -1, -1, -1}}, // 3
  {  3, { -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1}}, // 4
  {  4, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1}}, // 5
  {  5, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 6
  {  6, { -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 7
  {  7, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 8
  {  8, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 9
  {  9, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 10
  { 10, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 11
  { 11, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 12
  { 12, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 13
  { 13, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 14
  { 14, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 15
  { 15, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 16
  { 16, { -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 17
  { 17, { -1, -1, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 18
  { 19, { -1, -1, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 19
  { 20, { -1, -1, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 20
  { 21, { -1, -1, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 21
  { 22, { -1, -1, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 22
  { 24, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 23
  { 25, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 24
  { 26, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 25
  { 27, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 26
  { 29, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 27
  { 30, { -1, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 28
  { 31, { 38, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 29
  { 33, { 38, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 30
  { 34, { 38, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 31
  { 35, { 38, 37, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1}}, // 32
  {  8, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1, -1}}, // 33
  {  9, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 34
  { 10, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 35
  { 11, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 36
  { 12, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 37
  { 13, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 38
  { 14, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 39
  { 15, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 40
  { 16, { 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 41
  { 17, { -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 42
  { 18, { -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 43
  { 19, { -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 44
  { 20, { -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 45
  { 21, { -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15}}, // 46
  { 22, { -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 47
  { 23, { -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 48
  { 24, { -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 49
  { 25, { -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 50
  { 26, { -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, -1}}, // 51
  { 27, { -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 52
  { 28, { -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 53
  { 29, { -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 54
  { 30, { -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1, -1}}, // 55
  { 31, { -1, -1, -1, -1, -1, -1, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1, -1}}, // 56
  { 32, { -1, -1, -1, -1, -1, -1, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1, -1}}, // 57
  { 33, { -1, -1, -1, -1, -1, -1, -1, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1, -1}}, // 58
  { 34, { -1, -1, -1, -1, -1, -1, -1, -1, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1, -1}}, // 59
  { 35, { -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, 25, 24, 23, 22, 21, 20, -1, -1, -1, -1, -1}}  // 60
};//       1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21

const int ROWS = sizeof(sole) / sizeof(*sole);
