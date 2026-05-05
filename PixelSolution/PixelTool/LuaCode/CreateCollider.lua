--#BEGIN_TABLE Circle2DCollider
local Circle2DColliderTable = 
{
	Key = "KeyName",
    ColliderType = ColliderType.Circle2D,
    Center = {x = 0, y = 0},
    Radius = 0.5,
}
--#END_TABLE Circle2DCollider


--#BEGIN_TABLE Box2DCollider
local Box2DColliderTable = 
{
	Key = "KeyName",
    ColliderType = ColliderType.Box2D,
    Center = {x = 0, y = 0},
    Offset = {x = 0.5, y = 0.5},
}
--#END_TABLE Box2DCollider


--#BEGIN_TABLE RigidBody
local RigidBodyTable = 
{
       Active = true,
       Gravity = 1.0,          --(0.0 ~ 1.0)
       AutoSleep = true,
       Sensor = false,
       Restitution = 0.1,      --(0.0 ~ 1.0)
       Friction = 0.2,         --(0.0 ~ 5.0)
       LinearDamping = 0.05,   --(0.0 ~ 10.0)
       LockPositionX = false,
       LockPositionY = false,
       LockPositionZ = true,
       LockRotationX = true,
       LockRotationY = true,
       LockRotationZ = true,
       ColliderMotionType = ColliderMotionType.Dynamic,
}
--#END_TABLE RigidBody