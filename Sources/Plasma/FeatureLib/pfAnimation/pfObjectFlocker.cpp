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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plRefMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "hsTimer.h"
#include "../plMath/plRandom.h"
#include "../pnMessage/plEnableMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plLoadCloneMsg.h"

//#include "../plPipeline/plDebugGeometry.h"

#include <math.h>
#include <algorithm>

#include "pfObjectFlocker.h"

#define PI        3.14159f
#define HALF_PI   (PI/2)
#define GRAVITY   9.806650f  // meters/second
#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY  999999.0f

#define RAND() (float) (rand()/(RAND_MAX * 1.0))
#define SIGN(x) (((x) < 0) ? -1 : 1)

const int pfObjectFlocker::fFileVersion = 1;

#define FLOCKER_SHOW_DEBUG_LINES 0

#if FLOCKER_SHOW_DEBUG_LINES
// make a few easy-to-use colors for the debug lines
#define DEBUG_COLOR_RED			255,   0,   0
#define DEBUG_COLOR_GREEN		  0, 255,   0
#define DEBUG_COLOR_BLUE		  0,   0, 255
#define DEBUG_COLOR_YELLOW		255, 255,   0
#define DEBUG_COLOR_CYAN		  0, 255, 255
#define DEBUG_COLOR_PINK		255,   0, 255
#endif

///////////////////////////////////////////////////////////////////////////////
// Some quick utility functions
///////////////////////////////////////////////////////////////////////////////

// Basic linear interpolation
template<class T>
inline T Interpolate(float alpha, const T& x0, const T& x1)
{
	return x0 + ((x1 - x0) * alpha);
}

// Clip a value to the min and max, if necessary
inline float Clip(const float x, const float min, const float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline float ScalarRandomWalk(const float initial, const float walkspeed, const float min, const float max)
{
	const float next = initial + (((RAND() * 2) - 1) * walkspeed);
	if (next < min) return min;
	if (next > max) return max;
	return next;
}

// Classify a value relative to the interval between two bounds:
//     returns -1 when below the lower bound
//     returns  0 when between the bounds (inside the interval)
//     returns +1 when above the upper bound
inline int IntervalComparison (float x, float lowerBound, float upperBound)
{
	if (x < lowerBound) return -1;
	if (x > upperBound) return +1;
	return 0;
}

// Blend the new value into the accumulator using the smooth rate
// If smoothRate is 0 the accumulator will not change.
// If smoothRate is 1 the accumulator will be set to the new value with no smoothing.
// Useful values are "near zero".
template<class T>
inline void BlendIntoAccumulator(const float smoothRate, const T &newValue, T &smoothedAccumulator)
{
	smoothedAccumulator = Interpolate(Clip(smoothRate, 0, 1), smoothedAccumulator, newValue);
}

// return component of vector parallel to a unit basis vector
// (IMPORTANT NOTE: assumes "basis" has unit magnitude (length==1))
inline hsVector3 ParallelComponent(const hsVector3 &vec, const hsVector3 &unitBasis)
{
	const float projection = vec * unitBasis;
	return unitBasis * projection;
}

// return component of vector perpendicular to a unit basis vector
// (IMPORTANT NOTE: assumes "basis" has unit magnitude (length==1))
inline hsVector3 PerpendicularComponent(const hsVector3 &vec, const hsVector3& unitBasis)
{
	return vec - ParallelComponent(vec, unitBasis);
}

// clamps the length of a given vector to maxLength.  If the vector is
// shorter its value is returned unaltered, if the vector is longer
// the value returned has length of maxLength and is parallel to the
// original input.
hsVector3 TruncateLength (const hsVector3 &vec, const float maxLength)
{
	const float maxLengthSquared = maxLength * maxLength;
	const float vecLengthSquared = vec.MagnitudeSquared();
	if (vecLengthSquared <= maxLengthSquared)
		return vec;
	else
		return vec * (maxLength / sqrt(vecLengthSquared));
}

// Enforce an upper bound on the angle by which a given arbitrary vector
// diviates from a given reference direction (specified by a unit basis
// vector).  The effect is to clip the "source" vector to be inside a cone
// defined by the basis and an angle.
hsVector3 LimitMaxDeviationAngle(const hsVector3 &vec, const float cosineOfConeAngle, const hsVector3 &basis)
{
	// immediately return zero length input vectors
	float sourceLength = vec.Magnitude();
	if (sourceLength == 0) return vec;

	// measure the angular diviation of "source" from "basis"
	const hsVector3 direction = vec / sourceLength;
	float cosineOfSourceAngle = direction * basis;

	// Simply return "source" if it already meets the angle criteria.
	if (cosineOfSourceAngle >= cosineOfConeAngle) return vec;

	// find the portion of "source" that is perpendicular to "basis"
	const hsVector3 perp = PerpendicularComponent(vec, basis);

	// normalize that perpendicular
	hsVector3 unitPerp = perp;
	unitPerp.Normalize();

	// construct a new vector whose length equals the source vector,
	// and lies on the intersection of a plane (formed the source and
	// basis vectors) and a cone (whose axis is "basis" and whose
	// angle corresponds to cosineOfConeAngle)
	float perpDist = sqrt(1 - (cosineOfConeAngle * cosineOfConeAngle));
	const hsVector3 c0 = basis * cosineOfConeAngle;
	const hsVector3 c1 = unitPerp * perpDist;
	return (c0 + c1) * sourceLength;
}

///////////////////////////////////////////////////////////////////////////////
// pfVehicle functions
///////////////////////////////////////////////////////////////////////////////

void pfVehicle::IMeasurePathCurvature(const float elapsedTime)
{
	if (elapsedTime > 0)
	{
		const hsVector3 deltaPosition(&fLastPos, &Position());
		const hsVector3 deltaForward = (fLastForward - Forward()) / deltaPosition.Magnitude();
		const hsVector3 lateral = PerpendicularComponent(deltaForward, Forward());
		const float sign = ((lateral * Side()) < 0) ? 1.0f : -1.0f;
		fCurvature = lateral.Magnitude() * sign;
		BlendIntoAccumulator(elapsedTime * 4.0f, fCurvature, fSmoothedCurvature);
		fLastForward = Forward();
		fLastPos = Position();
	}
}

// Reset functions
void pfVehicle::Reset()
{
	ResetLocalSpace();

	SetMass(1); // defaults to 1 so acceleration = force
	SetSpeed(0); // speed along the forward direction

	SetMaxForce(10.0f); // steering force is clipped to this magnitude
	SetMaxSpeed(5.0f); // velocity is clipped to this magnitude

	SetRadius(0.5f); // size of bounding sphere

	// Reset bookkeeping for our averages
	ResetSmoothedPosition();
	ResetSmoothedCurvature();
	ResetSmoothedAcceleration();
}

float pfVehicle::ResetSmoothedCurvature(float value /* = 0 */)
{
	fLastForward.Set(0, 0, 0);
	fLastPos.Set(0, 0, 0);
	return fSmoothedCurvature = fCurvature = value;
}

hsVector3 pfVehicle::ResetSmoothedAcceleration(const hsVector3 &value /* = hsVector3(0,0,0) */)
{
	return fSmoothedAcceleration = value;
}

hsPoint3 pfVehicle::ResetSmoothedPosition(const hsPoint3 &value /* = hsPoint3(0,0,0) */)
{
	return fSmoothedPosition = value;
}

void pfVehicle::ResetLocalSpace()
{
	fSide.Set(1, 0, 0);
	fForward.Set(0, 1, 0);
	fUp.Set(0, 0, 1);
	fPos.Set(0, 0, 0);
}

// Geometry functions
void pfVehicle::SetUnitSideFromForwardAndUp()
{
	// derive new unit side vector from forward and up
	fSide = fForward % fUp;
	fSide.Normalize();
}

void pfVehicle::RegenerateOrthonormalBasisUF(const hsVector3 &newUnitForward)
{
	fForward = newUnitForward;

	// derive new side vector from NEW forward and OLD up
	SetUnitSideFromForwardAndUp();

	// derive new up vector from new side and new forward (should have unit length since side and forward are
	// perpendicular and unit length)
	fUp = fSide % fForward;
}

void pfVehicle::RegenerateLocalSpace(const hsVector3 &newVelocity, const float /*elapsedTime*/)
{
	// adjust orthonormal basis vectors to be aligned with new velocity
	if (Speed() > 0)
		RegenerateOrthonormalBasisUF(newVelocity / Speed());
}

void pfVehicle::RegenerateLocalSpaceForBanking(const hsVector3 &newVelocity, const float elapsedTime)
{
	// the length of this global-upward-pointing vector controls the vehicle's
	// tendency to right itself as it is rolled over from turning acceleration
	const hsVector3 globalUp(0, 0, 0.2f);

	// acceleration points toward the center of local path curvature, the
	// length determines how much the vehicle will roll while turning
	const hsVector3 accelUp = fSmoothedAcceleration * 0.05f;

	// combined banking, sum of up due to turning and global up
	const hsVector3 bankUp = accelUp + globalUp;

	// blend bankUp into vehicle's up vector
	const float smoothRate = elapsedTime * 3;
	hsVector3 tempUp = Up();
	BlendIntoAccumulator(smoothRate, bankUp, tempUp);
	tempUp.Normalize();
	SetUp(tempUp);

	// adjust orthonormal basis vectors to be aligned with new velocity
	if (Speed() > 0)
		RegenerateOrthonormalBasisUF(newVelocity / Speed());
}

// Physics functions
void pfVehicle::ApplySteeringForce(const hsVector3 &force, const float deltaTime)
{
	const hsVector3 adjustedForce = AdjustRawSteeringForce(force, deltaTime);

	// enforce limit on magnitude of steering force
	const hsVector3 clippedForce = TruncateLength(adjustedForce, MaxForce());

#if FLOCKER_SHOW_DEBUG_LINES
	// Draw the adjusted force vector
	plDebugGeometry::Instance()->DrawLine(Position(), Position() + clippedForce, DEBUG_COLOR_GREEN);
#endif

	// compute acceleration and velocity
	hsVector3 newAcceleration = (clippedForce / Mass());
	hsVector3 newVelocity = Velocity();

	// damp out abrupt changes and oscillations in steering acceleration
	// (rate is proportional to time step, then clipped into useful range)
	if (deltaTime > 0)
	{
		const float smoothRate = Clip(9 * deltaTime, 0.15f, 0.4f);
		BlendIntoAccumulator(smoothRate, newAcceleration, fSmoothedAcceleration);
	}

	// Euler integrate (per frame) acceleration into velocity
	newVelocity += fSmoothedAcceleration * deltaTime;

	// enforce speed limit
	newVelocity = TruncateLength(newVelocity, MaxSpeed());

	// update Speed
	SetSpeed(newVelocity.Magnitude());

	// Euler integrate (per frame) velocity into position
	SetPosition(Position() + (newVelocity * deltaTime));

	// regenerate local space (by default: align vehicle's forward axis with
	// new velocity, but this behavior may be overridden by derived classes.)
	RegenerateLocalSpace(newVelocity, deltaTime);

	// maintain path curvature information
	IMeasurePathCurvature(deltaTime);

	// running average of recent positions
	BlendIntoAccumulator(deltaTime * 0.06f, Position(), fSmoothedPosition);
}

hsVector3 pfVehicle::AdjustRawSteeringForce(const hsVector3 &force, const float deltaTime)
{
	const float maxAdjustedSpeed = 0.2f * MaxSpeed();

	if ((Speed() > maxAdjustedSpeed) || (force == hsVector3(0,0,0)))
		return force; // no adjustment needed if they are going above 20% of max speed
	else
	{
		const float range = Speed() / maxAdjustedSpeed; // make sure they don't turn too much if below 20% of max speed
		const float cosine = Interpolate(pow(range, 20), 1.0f, -1.0f);
		return LimitMaxDeviationAngle(force, cosine, Forward());
	}
}

void pfVehicle::ApplyBrakingForce(const float rate, const float deltaTime)
{
	const float rawBraking = Speed() * rate;
	const float clipBraking = ((rawBraking < MaxForce()) ? rawBraking : MaxForce());

	SetSpeed(Speed() - (clipBraking * deltaTime));
}

hsPoint3 pfVehicle::PredictFuturePosition(const float predictionTime)
{
	return Position() + (Velocity() * predictionTime);
}

///////////////////////////////////////////////////////////////////////////////
// pfBoidGoal functions
///////////////////////////////////////////////////////////////////////////////

pfBoidGoal::pfBoidGoal()
{
	fLastPos.Set(0, 0, 0);
	fCurPos.Set(0, 0, 0);
	fSpeed = 0;
	fHasLastPos = false; // our last pos doesn't make sense yet
}

void pfBoidGoal::Update(plSceneObject *goal, float deltaTime)
{
	if (!fHasLastPos) // the last pos is invalid, so we need to init now
	{
		fLastPos = fCurPos = goal->GetLocalToWorld().GetTranslate();
		fSpeed = 0;
		fForward.Set(1,0,0); // make a unit vector, it shouldn't matter that it's incorrect as this is only for one frame
		fHasLastPos = true;
		return;
	}

	fLastPos = fCurPos;
	fCurPos = goal->GetLocalToWorld().GetTranslate();
	hsVector3 change(&fCurPos, &fLastPos);
	float unadjustedSpeed = change.Magnitude();
	fSpeed = unadjustedSpeed / deltaTime; // update speed (mag is in meters, time is in seconds)
	if (unadjustedSpeed == 0)
		return; // if our speed is zero, don't recalc our forward vector (leave it as it was last time)
	fForward = change / unadjustedSpeed;

#if FLOCKER_SHOW_DEBUG_LINES
	// Show where we are predicting the location to be in 1 second
	plDebugGeometry::Instance()->DrawLine(Position(), PredictFuturePosition(1), DEBUG_COLOR_BLUE);
#endif
}

hsPoint3 pfBoidGoal::PredictFuturePosition(const float predictionTime)
{
	return fCurPos + (fForward * fSpeed * predictionTime);
}

///////////////////////////////////////////////////////////////////////////////
// pfBoid functions
///////////////////////////////////////////////////////////////////////////////

pfBoid::pfBoid(pfProximityDatabase& pd, pfObjectFlocker *flocker, plKey &key)
{
	// allocate a token for this boid in the proximity database
	fProximityToken = NULL;
	ISetupToken(pd);

	Reset();

	fFlockerPtr = flocker;
	fObjKey = key;

	IFlockDefaults();

	fProximityToken->UpdateWithNewPosition(Position());
}

pfBoid::pfBoid(pfProximityDatabase& pd, pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos)
{
	// allocate a token for this boid in the proximity database
	fProximityToken = NULL;
	ISetupToken(pd);

	Reset();

	fFlockerPtr = flocker;
	fObjKey = key;

	SetPosition(pos);

	IFlockDefaults();

	fProximityToken->UpdateWithNewPosition(Position());
}

pfBoid::pfBoid(pfProximityDatabase& pd, pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos, float speed, hsVector3 &forward, hsVector3 &side, hsVector3 &up)
{
	// allocate a token for this boid in the proximity database
	fProximityToken = NULL;
	ISetupToken(pd);

	Reset();

	fFlockerPtr = flocker;
	fObjKey = key;

	SetPosition(pos);
	SetSpeed(speed);
	SetForward(forward);
	SetSide(side);
	SetUp(up);

	IFlockDefaults();

	fProximityToken->UpdateWithNewPosition(Position());
}

pfBoid::~pfBoid()
{
	// delete this boid's token in the proximity database
	delete fProximityToken;

	plLoadCloneMsg* msg = TRACKED_NEW plLoadCloneMsg(fObjKey, fFlockerPtr->GetKey(), 0, false);
	msg->Send();
}

void pfBoid::IFlockDefaults()
{
	fSeparationRadius = 5.0f;
	fSeparationAngle = -0.707f;
	fSeparationWeight = 12.0f;

	fCohesionRadius = 9.0f;
	fCohesionAngle = -0.15f;
	fCohesionWeight = 8.0f;

	fGoalWeight = 8.0f;
	fRandomWeight = 12.0f;
}

void pfBoid::ISetupToken(pfProximityDatabase &pd)
{
	// delete this boid's token in the old proximity database
	delete fProximityToken;

	// allocate a token for this boid in the proximity database
	fProximityToken = pd.MakeToken(this);
}

hsBool pfBoid::IInBoidNeighborhood(const pfVehicle &other, const float minDistance, const float maxDistance, const float cosMaxAngle)
{
	if (&other == this) // abort if we're looking at ourselves
		return false;
	else
	{
		const hsVector3 offset(&(other.Position()), &Position());
		const float distanceSquared = offset.MagnitudeSquared();

		// definitely in neighborhood if inside minDistance sphere
		if (distanceSquared < (minDistance * minDistance))
			return true;
		else
		{
			// definitely not in neighborhood if outside maxDistance sphere
			if (distanceSquared > (maxDistance * maxDistance))
				return false;
			else
			{
				// otherwise, test angular offset from forward axis (can we "see" it?)
				const hsVector3 unitOffset = offset / sqrt(distanceSquared);
				const float forwardness = Forward() * unitOffset;
				return forwardness > cosMaxAngle;
			}
		}
	}
}

// Steering functions
hsVector3 pfBoid::ISteerForWander(float timeDelta)
{
	// random walk the fWanderSide and fWanderUp variables between -1 and +1
	const float speed = 10 * timeDelta; // the 10 value found through experimentation
	fWanderSide = ScalarRandomWalk(fWanderSide, speed, -1, +1);
	fWanderUp = ScalarRandomWalk(fWanderUp, speed, -1, +1);

	hsVector3 force = (Side() * fWanderSide) + (Up() * fWanderUp);

#if FLOCKER_SHOW_DEBUG_LINES
	// Draw the random walk component
	plDebugGeometry::Instance()->DrawLine(Position(), Position()+force, DEBUG_COLOR_PINK);
#endif

	return force;
}

hsVector3 pfBoid::ISteerForSeek(const hsPoint3 &target)
{
#if FLOCKER_SHOW_DEBUG_LINES
	// Draw to where we are steering towards
	plDebugGeometry::Instance()->DrawLine(Position(), target, DEBUG_COLOR_RED);
#endif

	const hsVector3 desiredVelocity(&target, &Position());
	return desiredVelocity - Velocity();
}

hsVector3 pfBoid::ISteerToGoal(pfBoidGoal &goal, float maxPredictionTime)
{
	// offset from this to quarry, that distance, unit vector toward quarry
	const hsVector3 offset(&goal.Position(), &Position());
	const float distance = offset.Magnitude();
	if (distance == 0) // nowhere to go
		return hsVector3(0, 0, 0);
	const hsVector3 unitOffset = offset / distance;

	// how parallel are the paths of "this" and the goal
	// (1 means parallel, 0 is pependicular, -1 is anti-parallel after later calculations)
	const float parallelness = Forward() * goal.Forward();

	// how "forward" is the direction to the quarry
	// (1 means dead ahead, 0 is directly to the side, -1 is straight back after later calculations)
	const float forwardness = Forward() * unitOffset;

	float speed = Speed();
	if (speed == 0)
		speed = 0.00001; // make it really small in case we start out not moving
	const float directTravelTime = distance / speed;
	const int f = IntervalComparison(forwardness,  -0.707f, 0.707f); // -1 if below -0.707f, 0 if between, and +1 if above 0.707f)
	const int p = IntervalComparison(parallelness, -0.707f, 0.707f); // 0.707 is basically cos(45deg) (45deg = PI/4)

	float timeFactor = 0; // to be filled in below - how far ahead to predict position so it looks good

	// Break the pursuit into nine cases, the cross product of the
	// quarry being [ahead, aside, or behind] us and heading
	// [parallel, perpendicular, or anti-parallel] to us.
	switch (f)
	{
	case +1:
		switch (p)
		{
		case +1: // ahead, parallel
			timeFactor = 4;
			break;
		case 0: // ahead, perpendicular
			timeFactor = 1.8f;
			break;
		case -1: // ahead, anti-parallel
			timeFactor = 0.85f;
			break;
		}
		break;
	case 0:
		switch (p)
		{
		case +1: // aside, parallel
			timeFactor = 1;
			break;
		case 0: // aside, perpendicular
			timeFactor = 0.8f;
			break;
		case -1: // aside, anti-parallel
			timeFactor = 4;
			break;
		}
		break;
	case -1:
		switch (p)
		{
		case +1: // behind, parallel
			timeFactor = 0.5f;
			break;
		case 0: // behind, perpendicular
			timeFactor = 2;
			break;
		case -1: // behind, anti-parallel
			timeFactor = 2;
			break;
		}
		break;
	}

	// estimated time until intercept of quarry
	const float et = directTravelTime * timeFactor;
	const float etl = (et > maxPredictionTime) ? maxPredictionTime : et;

	// estimated position of quarry at intercept
	const hsPoint3 target = goal.PredictFuturePosition(etl);

	// steer directly for our target (which is a point ahead of the object we are pursuing)
	return ISteerForSeek(target);
}

hsVector3 pfBoid::ISteerForSeparation(const float maxDistance, const float cosMaxAngle, const std::vector<pfVehicle*> &flock)
{
	// steering accumulator and count of neighbors, both initially zero
	hsVector3 steering(0, 0, 0);
	int neighbors = 0;

	// for each of the other vehicles...
	for (std::vector<pfVehicle*>::const_iterator other = flock.begin(); other != flock.end(); other++)
	{
		if (IInBoidNeighborhood(**other, Radius() * 3, maxDistance, cosMaxAngle))
		{
			// add in steering contribution
			// (opposite of the offset direction, divided once by distance
			// to normalize, divided another time to get 1/d falloff)
			const hsVector3 offset(&((**other).Position()), &Position());
			const float distanceSquared = offset * offset;
			steering += (offset / -distanceSquared);

			// count neighbors
			neighbors++;
		}
	}

	// divide by neighbors, then normalize to pure direction
	if (neighbors > 0)
	{
		steering = (steering / (float)neighbors);
		steering.Normalize();
	}

#if FLOCKER_SHOW_DEBUG_LINES
	// Draw the random walk component
	plDebugGeometry::Instance()->DrawLine(Position(), Position()+steering, DEBUG_COLOR_CYAN);
#endif

	return steering;
}

hsVector3 pfBoid::ISteerForCohesion(const float maxDistance, const float cosMaxAngle, const std::vector<pfVehicle*> &flock)
{
	// steering accumulator and count of neighbors, both initially zero
	hsVector3 steering(0, 0, 0);
	int neighbors = 0;

	// for each of the other vehicles...
	for (std::vector<pfVehicle*>::const_iterator other = flock.begin(); other != flock.end(); other++)
	{
		if (IInBoidNeighborhood(**other, Radius() * 3, maxDistance, cosMaxAngle))
		{
			// accumulate sum of neighbor's positions
			steering += (**other).Position();

			// count neighbors
			neighbors++;
		}
	}

	// divide by neighbors, subtract off current position to get error-
	// correcting direction, then normalize to pure direction
	if (neighbors > 0)
	{
		hsVector3 posVector(&(Position()), &(hsPoint3(0,0,0))); // quick hack to turn a point into a vector
		steering = ((steering / (float)neighbors) - posVector);
		steering.Normalize();
	}

#if FLOCKER_SHOW_DEBUG_LINES
	// Draw the random walk component
	plDebugGeometry::Instance()->DrawLine(Position(), Position()+steering, DEBUG_COLOR_YELLOW);
#endif

	return steering;
}

// Used for frame-by-frame updates; no time deltas on positions.
void pfBoid::Update(pfBoidGoal &goal, float deltaTime)
{
	const float maxTime = 20; // found by testing

	// find all flockmates within maxRadius using proximity database
	fNeighbors.clear();
	fProximityToken->FindNeighbors(Position(), fSeparationRadius, fNeighbors);

	hsVector3 goalVector = ISteerToGoal(goal, maxTime);
	hsVector3 randomVector = ISteerForWander(deltaTime);
	hsVector3 separationVector = ISteerForSeparation(fSeparationRadius, fSeparationAngle, fNeighbors);
	hsVector3 cohesionVector = ISteerForCohesion(fCohesionRadius, fCohesionAngle, fNeighbors);

	hsVector3 steeringVector = (fGoalWeight * goalVector) + (fRandomWeight * randomVector) +
		(fSeparationWeight * separationVector) + (fCohesionWeight * cohesionVector);

	ApplySteeringForce(steeringVector, deltaTime);

	fProximityToken->UpdateWithNewPosition(Position());
}

void pfBoid::RegenerateLocalSpace(const hsVector3 &newVelocity, const float elapsedTime)
{
	RegenerateLocalSpaceForBanking(newVelocity, elapsedTime);
}

///////////////////////////////////////////////////////////////////////////////
// pfFlock functions
///////////////////////////////////////////////////////////////////////////////
pfFlock::pfFlock() :
fGoalWeight(8.0f),
fRandomWeight(12.0f),
fSeparationRadius(5.0f),
fSeparationWeight(12.0f),
fCohesionRadius(9.0f),
fCohesionWeight(8.0f),
fMaxForce(10.0f),
fMaxSpeed(5.0f),
fMinSpeed(4.0f)
{
	 fDatabase = TRACKED_NEW pfBasicProximityDatabase<pfVehicle*>();
}

pfFlock::~pfFlock()
{
	int flock_size = fBoids.size();
	for (int i = 0; i < flock_size; i++)
	{
		delete fBoids[i];
		fBoids[i] = nil;
	}
	fBoids.clear();

	delete fDatabase;
	fDatabase = NULL;
}

void pfFlock::SetGoalWeight(float goalWeight)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetGoalWeight(goalWeight);
	fGoalWeight = goalWeight;
}

void pfFlock::SetWanderWeight(float wanderWeight)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetWanderWeight(wanderWeight);
	fRandomWeight = wanderWeight;
}

void pfFlock::SetSeparationWeight(float weight)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetSeparationWeight(weight);
	fSeparationWeight = weight;
}

void pfFlock::SetSeparationRadius(float radius)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetSeparationRadius(radius);
	fSeparationRadius = radius;
}

void pfFlock::SetCohesionWeight(float weight)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetCohesionWeight(weight);
	fCohesionWeight = weight;
}

void pfFlock::SetCohesionRadius(float radius)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetCohesionRadius(radius);
	fCohesionRadius = radius;
}

void pfFlock::SetMaxForce(float force)
{
	for (int i = 0; i < fBoids.size(); i++)
		fBoids[i]->SetMaxForce(force);
	fMaxForce = force;
}

void pfFlock::SetMaxSpeed(float speed)
{
	for (int i = 0; i < fBoids.size(); i++)
	{
		float speedAdjust = (fMaxSpeed - fMinSpeed) * RAND();
		fBoids[i]->SetMaxSpeed(speed - speedAdjust);
	}
	fMaxSpeed = speed;
}

void pfFlock::SetMinSpeed(float minSpeed)
{
	fMinSpeed = minSpeed;
}

void pfFlock::Update(plSceneObject *goal, float deltaTime)
{
	// update the goal data
	fBoidGoal.Update(goal, deltaTime);

	// update the flock
	float delta = (deltaTime > 0.3f) ? 0.3f : deltaTime;
	std::vector<pfBoid*>::iterator i;

	for (i = fBoids.begin(); i != fBoids.end(); i++)
		(*i)->Update(fBoidGoal, delta);
}

void pfFlock::AddBoid(pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos)
{
	pfBoid *newBoid = TRACKED_NEW pfBoid(*fDatabase, flocker, key, pos);

	newBoid->SetGoalWeight(fGoalWeight);
	newBoid->SetWanderWeight(fRandomWeight);

	newBoid->SetSeparationWeight(fSeparationWeight);
	newBoid->SetSeparationRadius(fSeparationRadius);

	newBoid->SetCohesionWeight(fCohesionWeight);
	newBoid->SetCohesionRadius(fCohesionRadius);

	newBoid->SetMaxForce(fMaxForce);
	float speedAdjust = (fMaxSpeed - fMinSpeed) * RAND();
	newBoid->SetMaxSpeed(fMaxSpeed - speedAdjust);

	fBoids.push_back(newBoid);
}

pfBoid *pfFlock::GetBoid(int i)
{
	if (i >= 0 && i < fBoids.size())
		return fBoids[i];
	else
		return nil;
}

pfObjectFlocker::pfObjectFlocker() :
fUseTargetRotation(false),
fRandomizeAnimationStart(true),
fNumBoids(0)
{
}

pfObjectFlocker::~pfObjectFlocker()
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
}

void pfObjectFlocker::SetNumBoids(UInt8 val)
{
	fNumBoids = val;
}

hsBool pfObjectFlocker::MsgReceive(plMessage* msg)
{
	plInitialAgeStateLoadedMsg* loadMsg = plInitialAgeStateLoadedMsg::ConvertNoRef(msg);
	if (loadMsg)
	{
		plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
		pMsg->AddReceiver(fBoidKey);
		pMsg->SetCmd(plEnableMsg::kDrawable);
		pMsg->AddType(plEnableMsg::kDrawable);
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers | plMessage::kPropagateToChildren);
		pMsg->SetCmd(plEnableMsg::kDisable);
		pMsg->Send();

		hsPoint3 pos(fTarget->GetLocalToWorld().GetTranslate());
		for (int i = 0; i < fNumBoids; i++)
		{
			plLoadCloneMsg* cloneMsg = TRACKED_NEW plLoadCloneMsg(fBoidKey->GetUoid(), GetKey(), 0);
			plKey newKey = cloneMsg->GetCloneKey();
			cloneMsg->Send();

			hsScalar xAdjust = (2 * RAND()) - 1; // produces a random number between -1 and 1
			hsScalar yAdjust = (2 * RAND()) - 1;
			hsScalar zAdjust = (2 * RAND()) - 1;
			hsPoint3 boidPos(pos.fX + xAdjust, pos.fY + yAdjust, pos.fZ + zAdjust);
			fFlock.AddBoid(this, newKey, boidPos);
		}
		plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());

		return true;
	}

	plLoadCloneMsg* lcMsg = plLoadCloneMsg::ConvertNoRef(msg);
	if (lcMsg && lcMsg->GetIsLoading())
	{
		if (fRandomizeAnimationStart)
		{
			plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
			pMsg->SetSender(GetKey());
			pMsg->SetBCastFlag(plMessage::kPropagateToModifiers | plMessage::kPropagateToChildren);
			pMsg->AddReceiver( lcMsg->GetCloneKey() );
			pMsg->SetCmd(plAnimCmdMsg::kGoToPercent);
			pMsg->fTime = RAND();
			pMsg->Send();
		}
	}

	return plSingleModifier::MsgReceive(msg);
}

hsBool pfObjectFlocker::IEval(double secs, hsScalar del, UInt32 dirty)
{
	fFlock.Update(fTarget, del);

	plSceneObject* boidSO = nil;
	for (int i = 0; i < fNumBoids; i++)
	{
		pfBoid* boid = fFlock.GetBoid(i);
		boidSO = plSceneObject::ConvertNoRef(boid->GetKey()->VerifyLoaded());

		hsMatrix44 l2w;
		hsMatrix44 w2l;

		if (fUseTargetRotation)
			l2w = fTarget->GetLocalToWorld();
		else
		{
			l2w = boidSO->GetLocalToWorld();
			hsVector3 forward = boid->Forward();
			hsVector3 up = boid->Up();
			hsVector3 side = boid->Side();

			// copy the vectors over
			for(int i = 0; i < 3; i++)
			{
				l2w.fMap[i][0] = side[i];
				l2w.fMap[i][1] = forward[i];
				l2w.fMap[i][2] = up[i];
			}
		}

		hsScalarTriple pos = boid->Position();
		l2w.SetTranslate(&pos);
		l2w.GetInverse(&w2l);

		boidSO->SetTransform(l2w, w2l);
	}

	return true;
}


void pfObjectFlocker::SetTarget(plSceneObject* so)
{
	plSingleModifier::SetTarget(so);

	if( fTarget )
	{
		plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	}
}

void pfObjectFlocker::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	int version = s->ReadByte();
	hsAssert(version <= fFileVersion, "Flocker data is newer then client, please update your client");

	SetNumBoids(s->ReadByte());
	fBoidKey = mgr->ReadKey(s);

	fFlock.SetGoalWeight(s->ReadSwapScalar());
	fFlock.SetWanderWeight(s->ReadSwapScalar());

	fFlock.SetSeparationWeight(s->ReadSwapScalar());
	fFlock.SetSeparationRadius(s->ReadSwapScalar());

	fFlock.SetCohesionWeight(s->ReadSwapScalar());
	fFlock.SetCohesionRadius(s->ReadSwapScalar());

	fFlock.SetMaxForce(s->ReadSwapScalar());
	fFlock.SetMaxSpeed(s->ReadSwapScalar());
	fFlock.SetMinSpeed(s->ReadSwapScalar());

	fUseTargetRotation = s->ReadBool();
	fRandomizeAnimationStart = s->ReadBool();
}

void pfObjectFlocker::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);

	s->WriteByte(fFileVersion);

	s->WriteByte(fNumBoids);
	mgr->WriteKey(s, fBoidKey);

	s->WriteSwapScalar(fFlock.GoalWeight());
	s->WriteSwapScalar(fFlock.WanderWeight());

	s->WriteSwapScalar(fFlock.SeparationWeight());
	s->WriteSwapScalar(fFlock.SeparationRadius());

	s->WriteSwapScalar(fFlock.CohesionWeight());
	s->WriteSwapScalar(fFlock.CohesionRadius());

	s->WriteSwapScalar(fFlock.MaxForce());
	s->WriteSwapScalar(fFlock.MaxSpeed());
	s->WriteSwapScalar(fFlock.MinSpeed());

	s->WriteBool(fUseTargetRotation);
	s->WriteBool(fRandomizeAnimationStart);
}
