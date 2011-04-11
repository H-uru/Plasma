
struct HS_RECT_NAME {
	HS_RECT_TYPE		fLeft, fTop, fRight, fBottom;

	HS_RECT_TYPE		Width() const { return fRight - fLeft; }
	HS_RECT_TYPE		Height() const { return fBottom - fTop; }
	hsBool			IsEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

	void				SetEmpty() { fLeft = fTop = fRight = fBottom = 0; }
	HS_RECT_NAME*	Set(HS_RECT_TYPE left, HS_RECT_TYPE top, HS_RECT_TYPE right, HS_RECT_TYPE bottom)
				{
					fLeft = left; fTop = top; fRight = right; fBottom = bottom;
					return this;
				}
	HS_RECT_NAME*	Set(const HS_RECT_POINT* p1, const HS_RECT_POINT* p2)
				{
					if (p1->fX < p2->fX)
					{	fLeft		= p1->fX;
						fRight	= p2->fX;
					} else
					{	fLeft		= p2->fX;
						fRight	= p1->fX;
					}
					
					if (p1->fY < p2->fY)
					{	fTop		= p1->fY;
						fBottom	= p2->fY;
					} else
					{	fTop		= p2->fY;
						fBottom	= p1->fY;
					}
					return this;
				}
	HS_RECT_NAME*	Set(UInt32 count, const HS_RECT_POINT pts[])
				{
					if (count > 0)
					{	fLeft = fRight = pts[0].fX;
						fTop = fBottom = pts[0].fY;
						(void)this->Union(count - 1, &pts[1]);
					}
					return this;
				}

	hsBool		Contains(HS_RECT_TYPE x, HS_RECT_TYPE y) const
				{
					return x >= fLeft && x < fRight && y >= fTop && y < fBottom;
				}
	hsBool		Contains(const HS_RECT_POINT* p) const
				{
					return this->Contains(p->fX, p->fY);
				}
	hsBool		Contains(const HS_RECT_NAME* r) const
				{
					return fLeft <= r->fLeft && fTop <= r->fTop && fRight >= r->fRight && fBottom >= r->fBottom;
				}
	hsBool		Contains(HS_RECT_TYPE left, HS_RECT_TYPE top, HS_RECT_TYPE right, HS_RECT_TYPE bottom) const
				{
					return fLeft <= left && fTop <= top && fRight >= right && fBottom >= bottom;
				}
	HS_RECT_NAME*	Offset(HS_RECT_TYPE dx, HS_RECT_TYPE dy)
				{
					fLeft += dx; fTop += dy; fRight += dx; fBottom += dy;
					return this;
				}
	HS_RECT_NAME*	MoveTo(HS_RECT_TYPE x, HS_RECT_TYPE y)
				{
					this->fRight += x - this->fLeft;
					this->fBottom += y - this->fTop;
					this->fLeft = x;
					this->fTop = y;
					return this;
				}
	HS_RECT_NAME*	Inset(HS_RECT_TYPE dx, HS_RECT_TYPE dy)
				{
					fLeft += dx; fRight -= dx;
					fTop += dy; fBottom -= dy;
					return this;
				}

	HS_RECT_NAME*	UnionX(HS_RECT_TYPE x)
				{
					if (x < fLeft) fLeft = x; else
					if (x > fRight) fRight = x;
					return this;
				}
	HS_RECT_NAME*	UnionY(HS_RECT_TYPE y)
				{
					if (y < fTop) fTop = y; else
					if (y > fBottom) fBottom = y;
					return this;
				}
	HS_RECT_NAME*	Union(const HS_RECT_NAME* r)
				{
					if (r->fLeft < fLeft)		fLeft = r->fLeft;
					if (r->fTop < fTop)		fTop = r->fTop;
					if (r->fRight > fRight)	fRight = r->fRight;
					if (r->fBottom > fBottom)	fBottom = r->fBottom;
					return this;
				}
	HS_RECT_NAME*	Union(const HS_RECT_POINT* p)
				{
					if (p->fX < fLeft)	fLeft = p->fX;
					if (p->fX > fRight)	fRight = p->fX;
					if (p->fY < fTop)	fTop = p->fY;
					if (p->fY> fBottom)	fBottom = p->fY;
					return this;	
				}
	HS_RECT_NAME*	Union(UInt32 count, const HS_RECT_POINT p[])
				{
					HS_RECT_TYPE	left = this->fLeft;
					HS_RECT_TYPE	top = this->fTop;
					HS_RECT_TYPE	right = this->fRight;
					HS_RECT_TYPE	bottom = this->fBottom;
					
					for (; count > 0; ++p, --count)
					{	HS_RECT_TYPE	value = p->fX;
						if (value < left)			left = value;
						else if (value > right)	right = value;
						
						value = p->fY;
						if (value < top)			top = value;
						else if (value > bottom)	bottom = value;
					}
					return this->Set(left, top, right, bottom);
				}

#if 0 // Havok reeks
	friend int		operator==(const HS_RECT_NAME& a, const HS_RECT_NAME& b)
				{
					return	a.fLeft == b.fLeft && a.fTop == b.fTop &&
							a.fRight == b.fRight && a.fBottom == b.fBottom;
				}
	friend int		operator!=(const HS_RECT_NAME& a, const HS_RECT_NAME& b)
				{
					return !(a == b);
				}
#else // Havok reeks
	int		operator==(const HS_RECT_NAME& aa) const
				{
					return	aa.fLeft == fLeft && aa.fTop == fTop &&
							aa.fRight == fRight && aa.fBottom == fBottom;
				}
	int		operator!=(const HS_RECT_NAME& aa) const
				{
					return !(aa == *this);
				}
#endif // Havok reeks

				// Intersect Test
	friend int		operator&&(const HS_RECT_NAME& a, const HS_RECT_NAME& b)
				{
					return	a.fLeft < b.fRight && a.fRight > b.fLeft &&
							a.fTop < b.fBottom && a.fBottom > b.fTop;
				}

	hsBool		Intersect(const HS_RECT_NAME* r)
				{
					return this->Intersect(r->fLeft, r->fTop, r->fRight, r->fBottom);
				}
	hsBool		Intersect(HS_RECT_TYPE left, HS_RECT_TYPE top, HS_RECT_TYPE right, HS_RECT_TYPE bottom)
				{
					if (left < fRight && top < fBottom && fLeft < right && fTop < bottom)
					{	if (left > fLeft)			fLeft = left;
						if (top > fTop)			fTop = top;
						if (right < fRight)		fRight = right;
						if (bottom < fBottom)	fBottom = bottom;
						return true;
					}
					return false;
				}
	hsBool		Intersect(const HS_RECT_NAME* a, const HS_RECT_NAME* b)
				{
					if (a->fLeft < b->fRight && a->fTop < b->fBottom && b->fLeft < a->fRight && b->fTop < a->fBottom)
					{	*this = *b;
						if (a->fLeft > fLeft)		fLeft = a->fLeft;
						if (a->fTop > fTop)		fTop = a->fTop;
						if (a->fRight < fRight)	fRight = a->fRight;
						if (a->fBottom < fBottom)	fBottom = a->fBottom;
						return true;
					}
					return false;	// "this" is not changed
				}

	HS_RECT_POINT*	ToQuad(HS_RECT_POINT quad[4]) const
				{
					quad[0].fX = fLeft;	quad[0].fY = fTop;
					quad[1].fX = fRight;	quad[1].fY = fTop;
					quad[2].fX = fRight;	quad[2].fY = fBottom;
					quad[3].fX = fLeft;	quad[3].fY = fBottom;
					return quad;
				}

	hsBool		CornerTest(const HS_RECT_NAME* area,
						HS_RECT_POINT* hitPt = nil, HS_RECT_POINT* oppositePt = nil) const
				{
					if (area->Contains(fLeft, fTop))
					{	if (hitPt)	hitPt->Set(fLeft, fTop);
						if (oppositePt)	oppositePt->Set(fRight, fBottom);
						return true;
					}
					if (area->Contains(fLeft, fBottom))
					{	if (hitPt)	hitPt->Set(fLeft, fBottom);
						if (oppositePt)	oppositePt->Set(fRight, fTop);
						return true;
					}
					if (area->Contains(fRight, fTop))
					{	if (hitPt)	hitPt->Set(fRight, fTop);
						if (oppositePt)	oppositePt->Set(fLeft, fBottom);
						return true;
					}
					if (area->Contains(fRight, fBottom))
					{	if (hitPt)	hitPt->Set(fRight, fBottom);
						if (oppositePt)	oppositePt->Set(fLeft, fTop);
						return true;
					}
					return false;
				}
	hsBool		CornerTest(HS_RECT_POINT* pt, HS_RECT_TYPE tolerance,
						HS_RECT_POINT* hitPt = nil, HS_RECT_POINT* oppositePt = nil) const
				{
					HS_RECT_NAME area = {	pt->fX - tolerance, pt->fY - tolerance,
										pt->fX + tolerance, pt->fY + tolerance };

					return this->CornerTest(&area, hitPt, oppositePt);
				}

#if !(HS_RECT_EXTEND)
};
#endif

#undef HS_RECT_NAME
#undef HS_RECT_POINT
#undef HS_RECT_TYPE
#undef HS_RECT_EXTEND

