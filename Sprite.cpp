#include "Sprite.h"

using namespace std;

Sprite::Sprite(BitMap* bitmap) {
	this->bitmap = bitmap;
	SetRect(&position, 0, 0, bitmap->getWidth(), bitmap->getHeight());
	calcCollisionRect();
	velocity.x = velocity.y = 0;
	zOrder = 0;
	SetRect(&bounds, 0, 0, 640, 480);
	boundsAction = BA_STOP;
	hidden = false;
}

Sprite::Sprite(BitMap* bitmap, RECT& bounds, BOUNDSACTION boundsAction) {
	this->bitmap = bitmap;

	// Calculate available space for positioning the sprite
	int availableWidth = bounds.right - bounds.left - bitmap->getWidth();
	int availableHeight = bounds.bottom - bounds.top - bitmap->getHeight();

	// Ensure that available width and height are non-negative
	if (availableWidth <= 0 || availableHeight <= 0) {
		// Handle the error, such as by setting the sprite's position to a default location
		// or throwing an exception.
		// For example, setting the sprite's position to the top-left corner of the bounds:
		SetRect(&position, bounds.left, bounds.top, bounds.left + bitmap->getWidth(), bounds.top + bitmap->getHeight());
	}
	else {
		// Calculate random position within the available space
		int xPos = rand() % availableWidth + bounds.left;
		int yPos = rand() % availableHeight + bounds.top;

		// Set sprite's position
		SetRect(&position, xPos, yPos, xPos + bitmap->getWidth(), yPos + bitmap->getHeight());
	}

	// Calculate collision rectangle
	calcCollisionRect();

	// Set velocity, zOrder, bounds, boundsAction, and hidden
	velocity.x = velocity.y = 0;
	zOrder = 0;
	CopyRect(&(this->bounds), &bounds);
	this->boundsAction = boundsAction;
	hidden = false;
}

Sprite::Sprite(BitMap* bitmap, POINT position, POINT velocity, int zOrder,
	RECT& boundary, BOUNDSACTION ba) {
	this->bitmap = bitmap;
	SetRect(&(this->position), position.x, position.y, position.x + bitmap->getWidth(),
		position.y + bitmap->getHeight());
	calcCollisionRect();
	this->velocity.x = velocity.x;
	this->velocity.y = velocity.y;
	this->zOrder = zOrder;
	CopyRect(&bounds, &boundary);
	boundsAction = ba;
	hidden = false;
}

SPRITEACTION Sprite::Update() {
	POINT newPos, spriteSize, boundSize;
	newPos.x = position.left + velocity.x;
	newPos.y = position.top + velocity.y;
	spriteSize.x = position.right - position.left;
	spriteSize.y = position.bottom - position.top;
	boundSize.x = bounds.right - bounds.left;
	boundSize.y = bounds.bottom - bounds.top;

	if (boundsAction == BA_BOUNCE) {
		bool bounce = false;
		POINT newVelocity;
		newVelocity.x = velocity.x;
		newVelocity.y = velocity.y;
		if (newPos.x < bounds.left) {
			bounce = true;
			newPos.x = bounds.left;
			newVelocity.x = -newVelocity.x;
		}
		else if (newPos.x + bitmap->getWidth() > bounds.right) {
			bounce = true;
			newPos.x = bounds.right - bitmap->getWidth();
			newVelocity.x = -newVelocity.x;
		}

		if (newPos.y < bounds.top) {
			bounce = true;
			newPos.y = bounds.top;
			newVelocity.y = -newVelocity.y;
		}
		else if (newPos.y + bitmap->getHeight() > bounds.bottom) {
			bounce = true;
			newPos.y = bounds.bottom - bitmap->getHeight();
			newVelocity.y = -newVelocity.y;
		}
		
		if (bounce) {
			setVelocity(newVelocity);
		}

	}

	else if (boundsAction == BA_WRAP) {
		if (newPos.x + spriteSize.x < bounds.left) {
			newPos.x = bounds.right;
		}
		else if (newPos.x > bounds.right) {
			newPos.x = bounds.left - spriteSize.x;
		}

		if (newPos.y + spriteSize.y < bounds.top) {
			newPos.y = bounds.bottom;
		}
		else if (newPos.y > bounds.bottom) {
			newPos.y = bounds.top - spriteSize.y;
		}
	}

	else if (boundsAction == BA_DIE) {
		if (newPos.x + spriteSize.x < bounds.left ||
			newPos.x > bounds.right ||
			newPos.y + spriteSize.y < bounds.top ||
			newPos.y > bounds.bottom) {
			return SA_KILL;
		}
	}

//	if (boundsAction == BA_STOP) {
	else{
		if (newPos.x < bounds.left) {
			setVelocity(0, 0);
			newPos.x = bounds.left;
		}
		else if (newPos.x + spriteSize.x > bounds.right) {
			setVelocity(0, 0);
			newPos.x = bounds.right - spriteSize.x;
		}
		if (newPos.y < bounds.top) {
			setVelocity(0, 0);
			newPos.y = bounds.top;
		}
		else if (newPos.y + spriteSize.y > bounds.bottom) {
			setVelocity(0, 0);
			newPos.y = bounds.bottom - spriteSize.y;
		}
	}

	setPosition(newPos);
	return SA_NONE;
}

void Sprite::Draw(HDC hdc) {
	if (bitmap != NULL && !hidden) {
		bitmap->draw(hdc, position.left, position.top, true);
	}
}

bool Sprite::isPointInside(int x, int y) {
	POINT p;
	p.x = x;
	p.y = y;
	return PtInRect(&position, p);
}

void Sprite::setPosition(POINT p) {
	OffsetRect(&position, p.x - position.left, p.y - position.top);
	calcCollisionRect();
}

void Sprite::setPosition(int x, int y) {
	OffsetRect(&position, x - position.left, y - position.top);
	calcCollisionRect();
}

void Sprite::offSetPosition(int x, int y) {
	OffsetRect(&position, x, y);
	calcCollisionRect();
}

void Sprite::setVelocity(int x, int y) {
	velocity.x = x;
	velocity.y = y;
}

void Sprite::setVelocity(POINT p) {
	velocity.x = p.x;
	velocity.y = p.y;
}

void Sprite::setBounds(RECT& r) {
	CopyRect(&bounds, &r);
}

Sprite::~Sprite() {
	//nothing, yay!
}

bool Sprite::testCollision(Sprite* s) {
	RECT& test = s->getCollision();
	return collision.left <= test.right &&
		collision.right >= test.left &&
		collision.top <= test.bottom &&
		collision.bottom >= test.top;
}

void Sprite::calcCollisionRect() {
	// Calculate the half-width and half-height of the sprite
	int spriteWidth = position.right - position.left;
	int spriteHeight = position.bottom - position.top;
	int xShrink = spriteWidth / 2;
	int yShrink = spriteHeight / 2;

	// Copy the sprite's position to the collision rectangle
	CopyRect(&collision, &position);

	// Shrink the collision rectangle by half of the sprite's width and height
	InflateRect(&collision, -xShrink, -yShrink);
}
