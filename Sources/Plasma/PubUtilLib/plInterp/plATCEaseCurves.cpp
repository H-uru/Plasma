/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plAnimEaseTypes.h"
#include "plAnimTimeConvert.h"

///////////////////////////////////////////////////////////////////////////////////////////////

plATCEaseCurve *plATCEaseCurve::CreateEaseCurve(uint8_t type, float minLength, float maxLength, float length, 
                                                float startSpeed, float goalSpeed)
{
    if (type == plAnimEaseTypes::kConstAccel)
        return new plConstAccelEaseCurve(minLength, maxLength, length, startSpeed, goalSpeed);
    if (type == plAnimEaseTypes::kSpline)
        return new plSplineEaseCurve(minLength, maxLength, length, startSpeed, goalSpeed);

    return nil;
}

void plATCEaseCurve::RecalcToSpeed(float startSpeed, float goalSpeed, hsBool preserveRate /* = false */)
{
    float rate = 1;

    if (fSpeed == goalSpeed && fStartSpeed == startSpeed) // already there, no need to do anything
        return;

    if (preserveRate)
        rate = (fSpeed - fStartSpeed) / fLength;

    fStartSpeed = startSpeed;
    fSpeed = goalSpeed;

    if (preserveRate)
        SetLengthOnRate(rate);
}

void plATCEaseCurve::SetLengthOnRate(float rate)
{
    fLength = (fSpeed - fStartSpeed) / rate;
    if (fLength < 0)
        fLength = -fLength;
}

float plATCEaseCurve::GetMinDistance()
{
    if (fMinLength == 0)
        return 0;
    
    float oldLength = fLength;
    fLength = fMinLength;
    float result = PositionGivenTime(fMinLength);
    fLength = oldLength;
    return result;
}

float plATCEaseCurve::GetMaxDistance()
{
    if (fMaxLength == 0)
        return 0;

    float oldLength = fLength;
    fLength = fMaxLength;
    float result = PositionGivenTime(fMaxLength);
    fLength = oldLength;
    return result;
}

float plATCEaseCurve::GetNormDistance()
{
    if (fNormLength == 0)
        return 0;
    
    float oldLength = fLength;
    fLength = fNormLength;
    float result = PositionGivenTime(fNormLength);
    fLength = oldLength;
    return result;
}

void plATCEaseCurve::Read(hsStream *s, hsResMgr *mgr)
{
    plCreatable::Read(s, mgr);

    fMinLength = s->ReadLEScalar();
    fMaxLength = s->ReadLEScalar();
    fNormLength = fLength = s->ReadLEScalar();
    fStartSpeed = s->ReadLEScalar();
    fSpeed = s->ReadLEScalar();
    fBeginWorldTime = s->ReadLEDouble();
}

void plATCEaseCurve::Write(hsStream *s, hsResMgr *mgr)
{
    plCreatable::Write(s, mgr);

    s->WriteLEScalar(fMinLength);
    s->WriteLEScalar(fMaxLength);
    s->WriteLEScalar(fNormLength);
    s->WriteLEScalar(fStartSpeed);
    s->WriteLEScalar(fSpeed);
    s->WriteLEDouble(fBeginWorldTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////

plConstAccelEaseCurve::plConstAccelEaseCurve()
{
    fMinLength = fMaxLength = fNormLength = fLength = 1;
    fBeginWorldTime = 0;

    RecalcToSpeed(0, 1);
}

plConstAccelEaseCurve::plConstAccelEaseCurve(float minLength, float maxLength, float length, 
                                             float startSpeed, float goalSpeed)
{
    fMinLength = minLength;
    fMaxLength = maxLength;
    fNormLength = fLength = length; 
    fBeginWorldTime = 0;

    RecalcToSpeed(startSpeed, goalSpeed);
}

plATCEaseCurve *plConstAccelEaseCurve::Clone() const
{
    plConstAccelEaseCurve *curve = new plConstAccelEaseCurve;
    curve->fStartSpeed = fStartSpeed;
    curve->fMinLength = fMinLength;
    curve->fMaxLength = fMaxLength;
    curve->fNormLength = fNormLength;
    curve->fBeginWorldTime = fBeginWorldTime;
    curve->fLength = fLength;
    curve->fSpeed = fSpeed;

    return curve;
}

void plConstAccelEaseCurve::SetLengthOnDistance(float dist)
{
    fLength = 2 * dist / (fSpeed + fStartSpeed);
}

float plConstAccelEaseCurve::PositionGivenTime(float time) const
{
    return (float)(fStartSpeed * time + (0.5 * (fSpeed - fStartSpeed) / fLength) * time * time);
}

float plConstAccelEaseCurve::VelocityGivenTime(float time) const
{
    return fStartSpeed + ((fSpeed - fStartSpeed) / fLength) * time;
}

float plConstAccelEaseCurve::TimeGivenVelocity(float velocity) const
{
    return (velocity - fStartSpeed) / ((fSpeed - fStartSpeed) / fLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////

plSplineEaseCurve::plSplineEaseCurve()
{
    fMinLength = fMaxLength = fNormLength = fLength = 1;
    fBeginWorldTime = 0;

    RecalcToSpeed(0, 1);
}

plSplineEaseCurve::plSplineEaseCurve(float minLength, float maxLength, float length, 
                                     float startSpeed, float goalSpeed)
{
    fMinLength = minLength;
    fMaxLength = maxLength;
    fNormLength = fLength = length; 
    fBeginWorldTime = 0;

    RecalcToSpeed(startSpeed, goalSpeed);
}

plATCEaseCurve *plSplineEaseCurve::Clone() const
{
    plSplineEaseCurve *curve = new plSplineEaseCurve;
    curve->fStartSpeed = fStartSpeed;
    curve->fMinLength = fMinLength;
    curve->fMaxLength = fMaxLength;
    curve->fNormLength = fNormLength;
    curve->fBeginWorldTime = fBeginWorldTime;
    curve->fLength = fLength;
    curve->fSpeed = fSpeed;

    int i;
    for (i = 0; i < 4; i++)
        curve->fCoef[i] = fCoef[i];

    return curve;
}

void plSplineEaseCurve::RecalcToSpeed(float startSpeed, float goalSpeed, hsBool preserveRate /* = false */)
{
    plATCEaseCurve::RecalcToSpeed(startSpeed, goalSpeed, preserveRate);
    
    // These are greatly simplified because the in/out tangents are always zero
    // Note: "b" is always zero for the ease splines we're currently doing (and will remain that way
    //       so long as the initial acceleration is zero. Can optimize a bit of the eval math to take 
    //       advantage of this.
    float a, b, c, d;

    a = fStartSpeed;
    b = 0;
    c = -3 * fStartSpeed + 3 * fSpeed;
    d = 2 * fStartSpeed - 2 * fSpeed;

    fCoef[0] = a;
    fCoef[1] = b;
    fCoef[2] = c;
    fCoef[3] = d;
}

void plSplineEaseCurve::SetLengthOnDistance(float dist)
{
    float curDist = PositionGivenTime(fLength);

    fLength = fLength * dist / curDist;
}

float plSplineEaseCurve::PositionGivenTime(float time) const
{
    float t1, t2, t3, t4;
    t1 = time / fLength;
    t2 = t1 * t1;
    t3 = t2 * t1;
    t4 = t3 * t1;
    
    return fLength * (fCoef[0] * t1 + fCoef[1] * t2 / 2 + fCoef[2] * t3 / 3 + fCoef[3] * t4 / 4);
}

float plSplineEaseCurve::VelocityGivenTime(float time) const
{
    float t1, t2, t3;
    t1 = time / fLength;
    t2 = t1 * t1;
    t3 = t2 * t1;
    return fCoef[0] + fCoef[1] * t1 + fCoef[2] * t2 + fCoef[3] * t3;
}

float plSplineEaseCurve::TimeGivenVelocity(float velocity) const 
{
    // Code based off of Graphics Gems V, pp 11-12 and
    // http://www.worldserver.com/turk/opensource/FindCubicRoots.c.txt

    // Solving the equation: fCoef[0] + fCoef[1] * t + fCoef[2] * t^2 + fCoef[3] * t^3 - velocity = 0

    float root;
    float a = (fCoef[0] - velocity) / fCoef[3];
    float b = fCoef[1] / fCoef[3];
    float c = fCoef[2] / fCoef[3];

    float Q = (c * c - 3 * b) / 9;
    float R = (2 * c * c * c - 9 * c * b + 27 * a) / 54;
    float Q3 = Q * Q * Q;
    float D = Q3 - R * R;

    if (D >= 0) 
    {   
        // 3 roots, find the one in the range [0, 1]
        const float pi = 3.14159;
        double theta = acos(R / sqrt(Q3));
        double sqrtQ = sqrt(Q);

        root = (float)(-2 * sqrtQ * cos((theta + 4 * pi) / 3) - c / 3); // Middle root, most likely to match
        if (root < 0.f || root > 1.f)
        {
            root = (float)(-2 * sqrtQ * cos((theta + 2 * pi) / 3) - c / 3); // Lower root
            if (root < 0.f || root > 1.f)
            {
                root = (float)(-2 * sqrtQ * cos(theta / 3) - c / 3); // Upper root
            }
        }
    }
    else // One root to the equation (I don't expect this to happen for ease splines, but JIC)
    {
        double E = sqrt(-D) + pow(fabs(R), 1.f / 3.f);
        root = (float)((E + Q / E) - c / 3);
        if (R > 0)
            root = -root;
    }

    if (root < 0.f || root > 1.f)
    {
        hsAssert(false, "No valid root found while solving animation spline");
        // Either a bug, or a rare case of floating-point inaccuracy. Either way, guess
        // the proper root as either the start or end of the curve based on the velocity.
        float dStart = velocity - fStartSpeed;
        if (dStart < 0)
            dStart = -dStart;
        float dEnd = velocity - fSpeed;
        if (dEnd < 0)
            dEnd = -dEnd;

        root = (dStart < dEnd ? 0.f : 1.f);
    }

    return root * fLength;
}

void plSplineEaseCurve::Read(hsStream *s, hsResMgr *mgr)
{
    plATCEaseCurve::Read(s, mgr);

    fCoef[0] = s->ReadLEScalar();
    fCoef[1] = s->ReadLEScalar();
    fCoef[2] = s->ReadLEScalar();
    fCoef[3] = s->ReadLEScalar();
}

void plSplineEaseCurve::Write(hsStream *s, hsResMgr *mgr)
{
    plATCEaseCurve::Write(s, mgr);

    s->WriteLEScalar(fCoef[0]);
    s->WriteLEScalar(fCoef[1]);
    s->WriteLEScalar(fCoef[2]);
    s->WriteLEScalar(fCoef[3]);
}

