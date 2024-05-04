// MyECSTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#define MAXIMUM_ENTITIES 64

typedef unsigned char BitSet8;

enum class EComponentFlags
{
	Position = 1 << 0,
	Velocity = 1 << 1,
};

struct BitFlag
{
public:
	void SetFlag(const BitSet8 Flag) { m_Flag |= Flag; }
	void UnsetFlag(const BitSet8 Flag) { m_Flag &= ~Flag; }
	void FlipFlag(const BitSet8 Flag) { m_Flag ^= Flag; }

	bool HasFlag(const BitSet8& Flag) const { return (m_Flag & Flag) == Flag; }
	bool HasAnyFlag(const BitSet8& Flags) const { return (m_Flag & Flags) != 0; }
	bool HasRequiredFlags(const BitSet8& Flags) const { return (m_Flag & Flags) == m_Flag; }
	bool HasFlagsExact(const BitSet8& Flags) const { return m_Flag == Flags; }

	const BitSet8& GetFlag() const { return m_Flag; }

private:
	BitSet8 m_Flag = 0;
};

typedef struct
{
	int X;
	int Y;
} PositionComponent;

typedef struct
{
	int X;
	int Y;
} VelocityComponent;

struct EntityData
{
	BitFlag Signature;
	PositionComponent Position{};
	VelocityComponent Velocity{};
};

static void AddPositionComponent(EntityData& Entity, int X = 0, int Y = 0)
{
	Entity.Position.X = X;
	Entity.Position.Y = Y;
	Entity.Signature.SetFlag((int)EComponentFlags::Position);
}

static void RemovePositionComponent(EntityData& Entity)
{
	Entity.Signature.UnsetFlag((int)EComponentFlags::Position);
}


static void AddVelocityComponent(EntityData& Entity, int X = 0, int Y = 0)
{
	Entity.Velocity.X = X;
	Entity.Velocity.Y = Y;
	Entity.Signature.SetFlag((int)EComponentFlags::Velocity);
}

static void RemoveVelocityComponent(EntityData& Entity)
{
	Entity.Signature.UnsetFlag((int)EComponentFlags::Velocity);
}

class System
{
public:
	inline bool MatchesQuery(const BitFlag Key) const { return m_Query.HasRequiredFlags(Key.GetFlag()); }
	virtual void Tick(EntityData Entities[MAXIMUM_ENTITIES]) = 0;

protected:
	BitFlag m_Query;
};

class MovementSystem : public System
{
public:
	MovementSystem() {
		m_Query.SetFlag((int)EComponentFlags::Position);
		m_Query.SetFlag((int)EComponentFlags::Velocity);
	}

	void Tick(EntityData* Entity) override
	{
		for (size_t i = 0; i < MAXIMUM_ENTITIES; i++)
		{
			if (!MatchesQuery(Entity->Signature))
			{
				continue;
			}

			Entity->Position.X += Entity->Velocity.X;
			Entity->Position.Y += Entity->Velocity.Y;

			Entity++;
		}
	}
};

class CollisionSystem : public System
{
public:
	CollisionSystem()
	{
		m_Query.SetFlag((int)EComponentFlags::Position);
	}

	void Tick(EntityData* Entity) override
	{
		EntityData* First = Entity;
		CollidedThisTick = false;

		for (size_t i = 0; i < MAXIMUM_ENTITIES; i++)
		{
			if (!MatchesQuery(Entity->Signature))
			{
				continue;
			}

			EntityData* ToCompare = First;
			for (size_t i = 0; i < MAXIMUM_ENTITIES; i++)
			{

				if (!MatchesQuery(ToCompare->Signature) || ToCompare == Entity)
				{
					ToCompare++;
					continue;
				}

				if (ToCompare->Position.X == Entity->Position.X && ToCompare->Position.Y == Entity->Position.Y)
				{
					CollidedThisTick = true;
					return;
				}

				ToCompare++;
			}
		}
	}

	bool CollidedThisTick = false;
};


int main()
{
	EntityData Components[MAXIMUM_ENTITIES];
	int A = 0;
	int B = 1;
	
	MovementSystem MoveSys;
	CollisionSystem CollSys;

	AddPositionComponent(Components[A], 3, 3);
	AddVelocityComponent(Components[A], -1, -1);

	AddPositionComponent(Components[B], 0, 0);

	bool ShouldRun = true;
	while (ShouldRun)
	{
		MoveSys.Tick(Components);
		CollSys.Tick(Components);

		if (CollSys.CollidedThisTick)
		{
			break;
		}
	}

}
