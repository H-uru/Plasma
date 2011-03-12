struct HS_POINT2_NAME {
	HS_POINT2_TYPE	fX, fY;

	HS_POINT2_NAME&	Set(HS_POINT2_TYPE x, HS_POINT2_TYPE y)
	{
		fX = x;
		fY = y;
		return *this;
	}
	HS_POINT2_NAME& operator+=(const HS_POINT2_NAME& s)
	{
		this->fX += s.fX;
		this->fY += s.fY;
		return *this;
	}
	HS_POINT2_NAME& operator-=(const HS_POINT2_NAME& s)
	{
		this->fX -= s.fX;
		this->fY -= s.fY;
		return *this;
	}

#if 0 // Havok reeks
	friend int operator==(const HS_POINT2_NAME& s, const HS_POINT2_NAME& t)
	{
		return (s.fX == t.fX && s.fY == t.fY);
	}
	friend int operator!=(const HS_POINT2_NAME& s, const HS_POINT2_NAME& t)
	{
		return !(s == t);
	}
#else // Havok reeks
	int operator==(const HS_POINT2_NAME& ss) const
	{
		return (ss.fX == fX && ss.fY == fY);
	}
	int operator!=(const HS_POINT2_NAME& ss)
	{
		return !(ss == *this);
	}
#endif // Havok reeks
	friend HS_POINT2_NAME operator+(const HS_POINT2_NAME& s, const HS_POINT2_NAME& t)
	{
		HS_POINT2_NAME	result;
		result.Set(s.fX + t.fX, s.fY + t.fY);
		return result;
	}
	friend HS_POINT2_NAME operator-(const HS_POINT2_NAME& s, const HS_POINT2_NAME& t)
	{
		HS_POINT2_NAME	result;
		result.Set(s.fX - t.fX, s.fY - t.fY);
		return result;
	}
	friend HS_POINT2_NAME operator-(const HS_POINT2_NAME& s)
	{
		HS_POINT2_NAME	result = { -s.fX, -s.fY };
		return result;
	}

#undef HS_POINT2_NAME
#undef HS_POINT2_TYPE
