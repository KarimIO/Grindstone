#ifndef _DRAWABLE_H
#define _DRAWABLE_H

class Drawable {
public:
	virtual void shadowDraw() = 0;
	virtual void draw() = 0;
};

#endif