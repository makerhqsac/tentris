#ifndef shapes_h
#define shapes_h

#define swap(a, b) { COLOR t = a; a = b; b = t; }
struct COLOR {
  byte R;
  byte G;
  byte B;
};
bool operator==(const COLOR& lhs, const COLOR& rhs)
{
    return lhs.R == rhs.R && lhs.G == rhs.G && lhs.B == rhs.B;
}
bool operator!=(const COLOR& lhs, const COLOR& rhs)
{
    return !(lhs.R == rhs.R && lhs.G == rhs.G && lhs.B == rhs.B);
}

byte shapeRotations[] = { 2, 4, 4, 1, 2, 4, 2 };
const char shapeNames[SHAPE_COUNT] = {
  'I',
  'J',
  'L',
  'O',
  'S',
  'T',
  'Z'
};
const byte shapes[SHAPE_COUNT][4][4] = {
  { // SHAPE_I
    {
      B1000,
      B1000,
      B1000,
      B1000
    },
    {
      B0000,
      B1111,
      B0000,
      B0000
    }
  },
  { // SHAPE_J
    {
      B0000,
      B0100,
      B0100,
      B1100
    },
    {
      B0000,
      B0000,
      B1000,
      B1110
    },
    {
      B0000,
      B1100,
      B1000,
      B1000
    },
    {
      B0000,
      B0000,
      B1110,
      B0010
    }
  },
  { // SHAPE_L
    {
      B0000,
      B1000,
      B1000,
      B1100
    },
    {
      B0000,
      B0000,
      B1110,
      B1000
    },
    {
      B0000,
      B1100,
      B0100,
      B0100
    },
    {
      B0000,
      B0000,
      B0010,
      B1110
    }
  },
  { // SHAPE_O
    {
      B0000,
      B0000,
      B1100,
      B1100
    }
  },
  { // SHAPE_S
    {
      B0000,
      B1000,
      B1100,
      B0100
    },
    {
      B0000,
      B0000,
      B0110,
      B1100
    },
  },
  { // SHAPE_T
    {
      B0000,
      B1000,
      B1100,
      B1000
    },
    {
      B0000,
      B0000,
      B0100,
      B1110
    },
    {
      B0000,
      B0000,
      B1110,
      B0100
    },
    {
      B0000,
      B0100,
      B1100,
      B0100
    }
  },
  { // SHAPE_Z
    {
      B0000,
      B0100,
      B1100,
      B1000
    },
    {
      B0000,
      B0000,
      B1100,
      B0110
    }
  }
};

#endif
