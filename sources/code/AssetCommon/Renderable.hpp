#ifndef _RENDERABLE_H
#define _RENDERABLE_H

class Renderable {
public:
	virtual void shadowDraw() = 0;
	virtual void draw() = 0;
};

#endif