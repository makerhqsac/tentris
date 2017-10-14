#ifndef shapes_h
#define shapes_h

#define swap2(x,y,x2,y2) { COLOR t = getPixel(x,y); fillBlock(x,y,getPixel(x2,y2)); fillBlock(x2,y2,t); }

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

const byte numbers[11][7] = {
  { // zero
      B111111,
      B100001,
      B100001,
      B100001,
      B100001,
      B100001,
      B111111
  },
  { // 1
      B001000,
      B011000,
      B101000,
      B001000,
      B001000,
      B001000,
      B111110
  },

  { // 2
      B111111,
      B000001,
      B000001,
      B111111,
      B100000,
      B100000,
      B111111
  },

  { // 3
      B111111,
      B000001,
      B000001,
      B000111,
      B000001,
      B000001,
      B111111
  },

  { // 4
      B100001,
      B100001,
      B100001,
      B111111,
      B000001,
      B000001,
      B000001
  },

  { // 5
      B111111,
      B100000,
      B100000,
      B111111,
      B000001,
      B000001,
      B111111
  },

  { // 6
      B100000,
      B100000,
      B100000,
      B111111,
      B100001,
      B100001,
      B111111
  },

  { // 7
      B111111,
      B000001,
      B000010,
      B000100,
      B001000,
      B010000,
      B100000
  },

  { // 8
      B111111,
      B100001,
      B100001,
      B111111,
      B100001,
      B100001,
      B111111
  },

  { // 9
      B111111,
      B100001,
      B100001,
      B111111,
      B000001,
      B000001,
      B000001
  },

  { // 10
      B100000,
      B100000,
      B100000,
      B100000,
      B100000,
      B100000,
      B100000
  }
};


#endif
