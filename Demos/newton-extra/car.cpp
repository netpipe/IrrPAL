#include "stdafx.h"
#include <iostream>

#include "dNewton.h"

#define DEMO_GRAVITY -9.81f

dNewton* gNewton;

struct BasciCarParameters
{
	enum DifferentialType
	{
		m_4WD,
		m_RWD,
		m_FWD,
	};

	dFloat VEHICLE_MASS;
	dFloat ENGINE_MASS;
	dFloat TIRE_MASS;
	dFloat ENGINE_ARMATURE_RADIO;
	dFloat CLUTCH_FRICTION_TORQUE;
	dFloat STEER_ANGLE;
	dFloat BRAKE_TORQUE;
	dFloat COM_Y_OFFSET;
	dFloat TOP_SPEED_KMH;

	dFloat IDLE_TORQUE;
	dFloat IDLE_TORQUE_RPM;

	dFloat PEAK_TORQUE;
	dFloat PEAK_TORQUE_RPM;

	dFloat PEAK_HP;
	dFloat PEAK_HP_RPM;

	dFloat REDLINE_RPM;

	dFloat GEAR_1;
	dFloat GEAR_2;
	dFloat GEAR_3;
	dFloat REVERSE_GEAR;

	dFloat SUSPENSION_LENGTH;
	dFloat SUSPENSION_SPRING;
	dFloat SUSPENSION_DAMPER;
	dFloat LATERAL_STIFFNESS;
	dFloat LONGITUDINAL_STIFFNESS;
	dFloat ALIGNING_MOMENT_TRAIL;

	DifferentialType m_differentialType;
	dMatrix m_tireaLigment;
};

static BasciCarParameters basicCarParameters = 
{
	 900.0f,	// VEHICLE_MASS
	  80.0f,	// ENGINE_MASS
	  40.0f,	// TIRE_MASS
	  0.125f,	// ENGINE_ARMATURE_RADIO
	  1000.0f,	// CLUTCH_FRICTION_TORQUE
	  10.0f,	// STEER_ANGLE
	4000.0f,	// BRAKE_TORQUE
	  -0.2f,	// COM_Y_OFFSET
	120.0f,		// TIRE_TOP_SPEED_KMH
	 400.0f,	// IDLE_TORQUE
	500.0f,		// IDLE_TORQUE_RPM
	 300.0f,	// PEAK_TORQUE
	3000.0f,	// PEAK_TORQUE_RPM
	 190.0f,	// PEAK_HP
	4000.0f,	// PEAK_HP_RPM
	4500.0f,	// REDLINE_RPM
		2.5f,	// GEAR_1
		2.0f,	// GEAR_2
		1.5f,	// GEAR_3
		2.9f,	// REVERSE_GEAR
	   0.40f,	// SUSPENSION_LENGTH
	  100.0f,	// SUSPENSION_SPRING
	   10.0f,	// SUSPENSION_DAMPER
	  900.0f * DEMO_GRAVITY *  5.0f,		// LATERAL_STIFFNESS proportional to the vehicle weight
	  900.0f * DEMO_GRAVITY *  2.0f,		// LONGITUDINAL_STIFFNESS proportional to the vehicle weight
	   1.5f,	// ALIGNING_MOMENT_TRAIL
	   BasciCarParameters::m_4WD,

	   dGetIdentityMatrix(),
};

static dFloat VehicleHullShape0[][3] =  
{
	{-2.3f, 0.0f, -0.9f}, {-2.3f, 0.0f, 0.9f}, {2.3f, 0.0f, -0.9f}, {2.3f, 0.0f, 0.9f},
	{-2.1f, 0.7f, -0.9f}, {-2.1f, 0.7f, 0.9f}, {2.1f, 0.7f, -0.9f}, {2.1f, 0.7f, 0.9f},
};

static dFloat VehicleHullShape1[][3] =  
{
	{-1.5f, 0.0f, -0.9f}, {-1.5f, 0.0f, 0.9f}, {1.2f, 0.0f, -0.9f}, {1.2f, 0.0f, 0.9f},
	{-1.1f, 0.7f, -0.9f}, {-1.1f, 0.7f, 0.9f}, {0.8f, 0.7f, -0.9f}, {0.8f, 0.7f, 0.9f},
};


// add force and torque to rigid body
void  PhysicsApplyGravityForce (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	NewtonBodyGetMass (body, &mass, &Ixx, &Iyy, &Izz);

	dVector force (dVector (0.0f, 1.0f, 0.0f).Scale (mass * DEMO_GRAVITY));
	NewtonBodySetForce (body, &force.m_x);
}

void TransformCallback(const NewtonBody* body, const dFloat* matrix, int threadIndex)
{
}

class BasicCarEntity
{
	enum DrivingState
	{
		m_engineOff,
		m_engineIdle,
		m_engineStop,
		m_driveForward,
		m_preDriveForward,
		m_driveReverse,
		m_preDriveReverse,
	};

	public: 
	BasicCarEntity (CustomVehicleControllerManager* const manager, const dMatrix& location, const BasciCarParameters& parameters)
		:m_tireaLigmentMatrix (dYawMatrix(3.141592f * 90.0f / 180.0f))
		,m_controller(NULL)
		,m_gearUpKey (false)
		,m_gearDownKey (false)
		,m_reverseGear (false)
		,m_engineKeySwitch(false)
		,m_automaticTransmission(true)
		,m_drivingState(m_engineOff)
	{
		NewtonWorld* const world = gNewton->GetNewton();

		// create the vehicle collision shape
		NewtonCollision* const chassisCollision = CreateChassisCollision (world);
		
		// create the coordinate system 
		dMatrix chassisMatrix;
		chassisMatrix.m_front = dVector (1.0f, 0.0f, 0.0f, 0.0f);			// this is the vehicle direction of travel
		chassisMatrix.m_up	  = dVector (0.0f, 1.0f, 0.0f, 0.0f);			// this is the downward vehicle direction
		chassisMatrix.m_right = chassisMatrix.m_front * chassisMatrix.m_up;	// this is in the side vehicle direction (the plane of the wheels)
		chassisMatrix.m_posit = dVector (0.0f, 0.0f, 0.0f, 1.0f);

		// create a default vehicle controller
//m_controller = manager->CreateVehicle (chassisCollision, chassisMatrix, parameters.VEHICLE_MASS, PhysicsApplyGravityForce, this);
m_controller = manager->CreateVehicle (chassisCollision, chassisMatrix, parameters.VEHICLE_MASS, PhysicsApplyGravityForce, NULL);

		// get body the vehicle rigid body and set the Newton rigid body physics properties
		NewtonBody* const body = m_controller->GetBody();

		// set the user data
		NewtonBodySetUserData(body, this);

		// set the transform callback
		NewtonBodySetTransformCallback (body, TransformCallback);

		// set the standard force and torque call back
		NewtonBodySetForceAndTorqueCallback(body, PhysicsApplyGravityForce);

		// set the player matrix 
		NewtonBodySetMatrix(body, &location[0][0]);

		// destroy the collision helper shape 
		NewtonDestroyCollision(chassisCollision);

		// map the gear to a look up table: gear 0 is reverse, gea 1 is neutral, gear 1 is first, gear 2 is second and so on
		for (int i = 0; i < int ((sizeof (m_gearMap) / sizeof (m_gearMap[0]))); i ++) {
			m_gearMap[i] = i;
		}
		m_gearMap[0] = 1;
		m_gearMap[1] = 0;
	}

	~BasicCarEntity ()
	{
		((CustomVehicleControllerManager*)m_controller->GetManager())->DestroyController(m_controller);
	}

	void SetGearMap(CustomVehicleController::EngineController* const engine)
	{
		int start = engine->GetFirstGear();
		int count = engine->GetLastGear() - start;
		for (int i = 0; i < count; i++) {
			m_gearMap[i + 2] = start + i;
		}
		m_gearMap[0] = engine->GetNeutralGear();
		m_gearMap[1] = engine->GetReverseGear();
	}

	NewtonCollision* CreateChassisCollision (NewtonWorld* const world) const
	{
		dMatrix offset (dGetIdentityMatrix());
		offset.m_posit.m_y = 0.7f;

		NewtonCollision* const convex0 = NewtonCreateConvexHull (world, 8, &VehicleHullShape0[0][0], 3 * sizeof (dFloat), 0.001f, 0, NULL);
		NewtonCollision* const convex1 = NewtonCreateConvexHull (world, 8, &VehicleHullShape1[0][0], 3 * sizeof (dFloat), 0.001f, 0, &offset[0][0]);
		
		NewtonCollision* const collision = NewtonCreateCompoundCollision(world, 0);

		NewtonCompoundCollisionBeginAddRemove(collision);
		NewtonCompoundCollisionAddSubCollision (collision, convex0);
		NewtonCompoundCollisionAddSubCollision (collision, convex1);
		NewtonCompoundCollisionEndAddRemove (collision);	

		NewtonDestroyCollision (convex0);
		NewtonDestroyCollision (convex1);

		return collision;
	}

	// interpolate all skeleton transform
/* CHANGE
	virtual void InterpolateMatrix (DemoEntityManager& world, dFloat param)
	{
//		DemoEntity::InterpolateMatrix(world, param);
		if (m_controller) {
			for (dList<CustomVehicleController::BodyPart*>::dListNode* node = m_controller->GetFirstBodyPart()->GetNext(); node; node = m_controller->GetNextBodyPart(node)) {
				CustomVehicleController::BodyPart* const part = node->GetInfo();
				DemoEntity* const entPart = (DemoEntity*)part->GetUserData();
				entPart->InterpolateMatrix(world, param);
			}
		}
	}
*/

	CustomVehicleController::BodyPartTire* AddTire (const dVector& offset, dFloat width, dFloat radius, dFloat mass, dFloat steeringAngle, dFloat suspensionLength, dFloat suspensionSpring, dFloat suspensionDamper, dFloat lateralStiffness, dFloat longitudinalStiffness, dFloat aligningMomentTrail, const dMatrix& tireAligmentMatrix) 
	{
		NewtonBody* const body = m_controller->GetBody();

		// make the tire matrix from the offset and the body matrix
//		dMatrix tireMatrix (GetNextMatrix());
		dMatrix tireMatrix (dGetIdentityMatrix());
		tireMatrix.m_posit = offset;

		// add the visual representation of the is tire to as a child of the vehicle model 
		NewtonCollision*  const tireMeshGenerator = NewtonCreateChamferCylinder (NewtonBodyGetWorld(body), 0.5f, 1.0f, 0, NULL);
		NewtonCollisionSetScale (tireMeshGenerator, width, radius, radius);
		NewtonDestroyCollision (tireMeshGenerator);

		// add the tire to the vehicle
		CustomVehicleController::BodyPartTire::Info tireInfo;
		tireInfo.m_location = tireMatrix.m_posit;
		tireInfo.m_mass = mass;
		tireInfo.m_radio = radius;
		tireInfo.m_width = width;
		tireInfo.m_maxSteeringAngle = steeringAngle * 3.1416f / 180.0f;
		tireInfo.m_dampingRatio = suspensionDamper;
		tireInfo.m_springStrength = suspensionSpring;
		tireInfo.m_suspesionlenght = suspensionLength;
		tireInfo.m_lateralStiffness = lateralStiffness;
		tireInfo.m_longitudialStiffness = longitudinalStiffness;
		tireInfo.m_aligningMomentTrail = aligningMomentTrail;
//		tireInfo.m_userData = tireEntity;

		return m_controller->AddTire (tireInfo);
	}


	void UpdateTireTransforms()
	{
		//NewtonBody* const body = m_controller->GetBody();

		for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
			const CustomVehicleController::BodyPartTire* const part = &node->GetInfo();
			CustomVehicleController::BodyPart* const parent = part->GetParent();

			NewtonBody* const body = part->GetBody();
			NewtonBody* const parentBody = parent->GetBody();

			dMatrix matrix;
			dMatrix parentMatrix;
			NewtonBodyGetMatrix(body, &matrix[0][0]);
			NewtonBodyGetMatrix(parentBody, &parentMatrix[0][0]);

//			DemoEntity* const tirePart = (DemoEntity*)part->GetUserData();
			matrix = m_tireaLigmentMatrix * matrix * parentMatrix.Inverse();

			dQuaternion rot(matrix);
//			tirePart->SetMatrix(*scene, rot, matrix.m_posit);
		}
	}

	// create a simple vehicle 
	void BuidlBasicCar (const BasciCarParameters& parameters)
	{
		// step one: find the location of each tire, in the visual mesh and add them one by one to the vehicle controller 
		dFloat width = 0.35f;
		dFloat radius = 0.5f;

		// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
		m_controller->SetCenterOfGravity (dVector (0.0f, parameters.COM_Y_OFFSET, 0.0f, 0.0f)); 

		// add front tires
		CustomVehicleController::BodyPartTire* frontTires[2]; 
		dVector offset1 (1.5f, 0.0f, -1.0f, 1.0f);
		frontTires[0] = AddTire (offset1, width, radius, parameters.TIRE_MASS, parameters.STEER_ANGLE, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireaLigment);
		offset1 = dVector (1.5f, 0.0f, 1.0f, 1.0f);		
		frontTires[1] = AddTire (offset1, width, radius, parameters.TIRE_MASS, parameters.STEER_ANGLE, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireaLigment);

		// add rear tires
		CustomVehicleController::BodyPartTire* rearTires[2];
		dVector offset2 (-1.7f, 0.0f, -1.0f, 1.0f);
		rearTires[0] = AddTire (offset2, width, radius, parameters.TIRE_MASS, 0.0f, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireaLigment);
		offset2 = dVector (-1.7f, 0.0f, 1.0f, 1.0f);
		rearTires[1] = AddTire (offset2, width, radius, parameters.TIRE_MASS, 0.0f, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireaLigment);

		// add a steering Wheel
		CustomVehicleController::SteeringController* const steering = new CustomVehicleController::SteeringController (m_controller);
		steering->AddTire (frontTires[0]);
		steering->AddTire (frontTires[1]);
		m_controller->SetSteering(steering);

		// add all wheels brakes
		CustomVehicleController::BrakeController* const brakes = new CustomVehicleController::BrakeController (m_controller, parameters.BRAKE_TORQUE);
		for (int i = 0; i < 2; i ++) {
			brakes->AddTire (frontTires[i]);
			brakes->AddTire (rearTires[i]);
		}
		m_controller->SetBrakes(brakes);

		// add hand brakes
		CustomVehicleController::BrakeController* const handBrakes = new CustomVehicleController::BrakeController (m_controller, parameters.BRAKE_TORQUE);
		handBrakes->AddTire (rearTires[0]);
		handBrakes->AddTire (rearTires[1]);
		m_controller->SetHandBrakes (handBrakes);

		CustomVehicleController::EngineController::Differential8wd differential;
		switch (parameters.m_differentialType) 
		{
			case BasciCarParameters::m_RWD:
				differential.m_type = CustomVehicleController::EngineController::Differential::m_2wd;
				differential.m_axel.m_leftTire = rearTires[0];
				differential.m_axel.m_rightTire = rearTires[1];
				break;
			case BasciCarParameters::m_FWD:
				differential.m_type = CustomVehicleController::EngineController::Differential::m_2wd;
				differential.m_axel.m_leftTire = frontTires[0];
				differential.m_axel.m_rightTire = frontTires[1];
				break;

			case BasciCarParameters::m_4WD:
			default:
				differential.m_type = CustomVehicleController::EngineController::Differential::m_4wd;
				differential.m_axel.m_leftTire = rearTires[0];
				differential.m_axel.m_rightTire = rearTires[1];
				differential.m_secundAxel.m_axel.m_leftTire = frontTires[0];
				differential.m_secundAxel.m_axel.m_rightTire = frontTires[1];
		}

		CustomVehicleController::EngineController::Info engineInfo;
		engineInfo.m_mass = parameters.ENGINE_MASS;
		engineInfo.m_radio = parameters.ENGINE_ARMATURE_RADIO;
		engineInfo.m_vehicleTopSpeed = parameters.TOP_SPEED_KMH;
		engineInfo.m_clutchFrictionTorque = parameters.CLUTCH_FRICTION_TORQUE;
		
		engineInfo.m_idleTorque = parameters.IDLE_TORQUE;
		engineInfo.m_idleTorqueRpm = parameters.IDLE_TORQUE_RPM;

		engineInfo.m_peakTorque = parameters.PEAK_TORQUE;
		engineInfo.m_peakTorqueRpm = parameters.PEAK_TORQUE_RPM;

		engineInfo.m_peakHorsePower = parameters.PEAK_HP;
		engineInfo.m_peakHorsePowerRpm = parameters.PEAK_HP_RPM;

		engineInfo.m_readLineRpm = parameters.REDLINE_RPM;

		engineInfo.m_gearsCount = 3;
		engineInfo.m_gearRatios[0] = parameters.GEAR_1;
		engineInfo.m_gearRatios[1] = parameters.GEAR_2;
		engineInfo.m_gearRatios[2] = parameters.GEAR_3;
		engineInfo.m_reverseGearRatio = parameters.REVERSE_GEAR;

		CustomVehicleController::EngineController* const engineControl = new CustomVehicleController::EngineController (m_controller, engineInfo, differential);

		// the the default transmission type
		engineControl->SetTransmissionMode(m_automaticTransmission);

		m_controller->SetEngine(engineControl);

		// set the gear look up table
		SetGearMap(engineControl);

		// set teh vehicle weigh doistibution 
		m_controller->SetWeightDistribution (0.5f);

		// do not forget to call finalize after all components are added or after any change is made to the vehicle
		m_controller->Finalize();	
	}

	void ApplyPlayerControl ()
	{
		NewtonBody* const body = m_controller->GetBody();
		NewtonWorld* const world = NewtonBodyGetWorld(body);
//		DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(world);
//		NewtonDemos* const mainWindow = scene->GetRootWindow();

		CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
		CustomVehicleController::BrakeController* const brakes = m_controller->GetBrakes();
		CustomVehicleController::BrakeController* const handBrakes = m_controller->GetHandBrakes();
		CustomVehicleController::SteeringController* const steering = m_controller->GetSteering();

		int gear = engine->GetGear();
		int engineIgnitionKey = 0;
		int automaticTransmission = engine->GetTransmissionMode();
		dFloat cluthPedal = 1.0f;
		dFloat steeringVal = 0.0f;
		dFloat reverseGasPedal = 0.0f;
		dFloat forwardGasPedal = 0.0f;
		dFloat handBrakePedal = 0.0f;
		
		engineIgnitionKey = m_engineKeySwitch;
		automaticTransmission = m_automaticTransmission;
		steeringVal = 0.0f;
		gear += int(m_gearUpKey) - int(m_gearDownKey);

		bool b_Gas = false;
		bool b_Reverse = false;
		bool b_Handbrake = false;
		bool b_Cluth = false;

		if (b_Gas) {
			forwardGasPedal = 1.0f;
		}

		if (b_Reverse) {
			reverseGasPedal = 1.0f;
		}

		if (b_Handbrake) {
			handBrakePedal = 1.0f;
		}

		if (b_Cluth) {
 			cluthPedal = 0.0f;
		}

		steering->SetParam(steeringVal);
		switch (m_drivingState)
		{
			case m_engineOff:
			{
				if (engineIgnitionKey) {
					m_drivingState = m_engineIdle;
					engine->SetIgnition (true);
					handBrakes->SetParam(0.0f);
					engine->SetGear (engine->GetNeutralGear());
				} else {
					engine->SetIgnition (false);
					engine->SetGear(engine->GetFirstGear());
					handBrakes->SetParam(1.0f);
				}
				break;
			}

			case m_engineIdle:
			{
				brakes->SetParam(0.0f);
				handBrakes->SetParam(handBrakePedal);
				if (!engineIgnitionKey) {
					m_drivingState = m_engineOff;
				} else {
					if (forwardGasPedal) {
						m_drivingState = m_preDriveForward;
					} else if (reverseGasPedal) {
						m_drivingState = m_preDriveReverse;
					}
				}
				break;
			}

			case m_engineStop:
			{
				if (forwardGasPedal || reverseGasPedal) {
					brakes->SetParam(1.0f);
				} else {
					m_drivingState = m_engineIdle;
				}
				break;
			}

			case m_preDriveForward:
			{
				if (engine->GetSpeed() < -5.0f) {
					brakes->SetParam(0.5f);
					engine->SetClutchParam(0.0f);
					engine->SetGear(engine->GetNeutralGear());
				} else {
					m_drivingState = m_driveForward;
					engine->SetGear(engine->GetFirstGear());
				}
				break;
			}	

			case m_driveForward:
			{
				engine->SetParam(forwardGasPedal);
				engine->SetClutchParam(cluthPedal);
				handBrakes->SetParam(handBrakePedal);
				if (reverseGasPedal) {
					brakes->SetParam(reverseGasPedal);
					if (engine->GetSpeed() < 5.0f) {
						engine->SetGear(engine->GetNeutralGear());
						m_drivingState = m_engineStop;
					}
				} else {
					brakes->SetParam(0.0f);
				}

				if (!engineIgnitionKey) {
					m_drivingState = m_engineStop;
				}

				break;
			}

			case m_preDriveReverse:
			{
				if (engine->GetSpeed() > 5.0f) {
					brakes->SetParam(0.5f);
					engine->SetClutchParam(0.0f);
					engine->SetGear(engine->GetNeutralGear());
				} else {
					m_drivingState = m_driveReverse;
					engine->SetGear(engine->GetReverseGear());
				}
				break;
			}

			case m_driveReverse:
			{
				engine->SetParam(reverseGasPedal);
				engine->SetClutchParam(cluthPedal);
				handBrakes->SetParam(handBrakePedal);
				if (forwardGasPedal) {
					brakes->SetParam(forwardGasPedal);
					if (engine->GetSpeed() > -5.0f) {
						engine->SetGear(engine->GetNeutralGear());
						m_drivingState = m_engineStop;
					}
				} else {
					brakes->SetParam(0.0f);
				}

				if (!engineIgnitionKey) {
					m_drivingState = m_engineStop;
				}
				break;
			}
		}
	}

	void ApplyNPCControl ()
	{
//		dAssert (0);
	}

	dMatrix m_tireaLigmentMatrix;
	CustomVehicleController* m_controller;
	bool m_gearUpKey;
	bool m_gearDownKey;
	bool m_reverseGear;
	bool m_engineKeySwitch;
	bool m_automaticTransmission;
	/* CHANGE
	DemoEntityManager::ButtonKey m_gearUpKey;
	DemoEntityManager::ButtonKey m_gearDownKey;
	DemoEntityManager::ButtonKey m_reverseGear;
	DemoEntityManager::ButtonKey m_engineKeySwitch;
	DemoEntityManager::ButtonKey m_automaticTransmission;
	*/
	int m_gearMap[10];
	DrivingState m_drivingState;
};


class BasicCarControllerManager: public CustomVehicleControllerManager
{
	public:
	BasicCarControllerManager (NewtonWorld* const world, int materialsCount, int* const materialList)
		:CustomVehicleControllerManager (world, materialsCount, materialList)
		,m_player (NULL) 
	{
	}

	~BasicCarControllerManager ()
	{
	}

	void SetAsPlayer (BasicCarEntity* const player)
	{
		m_player = player;
	}

	virtual void PreUpdate (dFloat timestep)
	{
		// apply the vehicle controls, and all simulation time effect
		for (dListNode* ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
			CustomVehicleController* const controller = &ptr->GetInfo();

			NewtonBody* const body = controller->GetBody();
			BasicCarEntity* const vehicleEntity = (BasicCarEntity*) NewtonBodyGetUserData(body);

			if (vehicleEntity == m_player) {
				// do player control
				vehicleEntity->ApplyPlayerControl ();
			} else {
				// do no player control
				vehicleEntity->ApplyNPCControl ();
			}
		}

		// do the base class post update
		CustomVehicleControllerManager::PreUpdate(timestep);
	}

	virtual void PostUpdate (dFloat timestep)
	{
		// do the base class post update
		CustomVehicleControllerManager::PostUpdate(timestep);

		// update the visual transformation matrices for all vehicle tires
		for (dListNode* node = GetFirst(); node; node = node->GetNext()) {
			CustomVehicleController* const controller = &node->GetInfo();
			BasicCarEntity* const vehicleEntity = (BasicCarEntity*)NewtonBodyGetUserData (controller->GetBody());
			vehicleEntity->UpdateTireTransforms();
		}
	}

	BasicCarEntity* m_player;
};


int _tmain(int argc, _TCHAR* argv[])
{
	// create a newton world
	gNewton = new dNewton();
	NewtonWorld* const world = gNewton->GetNewton();

	dMatrix location (dGetIdentityMatrix());
	location.m_posit = dVector (0.0f, 10.0f, 0.0f, 1.0f);

	//location.m_posit = FindFloor (scene->GetNewton(), location.m_posit, 100.0f);
	location.m_posit.m_y += 2.0f;

	// create a vehicle controller manager
	int defaulMaterial = NewtonMaterialGetDefaultGroupID(world);
	int materialList[] = {defaulMaterial };
	BasicCarControllerManager* const manager = new BasicCarControllerManager (world, 1, materialList);
	
	// load 
	basicCarParameters.m_differentialType = BasciCarParameters::m_RWD;
	BasicCarEntity* const heavyVehicle = new BasicCarEntity (manager, location, basicCarParameters);
	heavyVehicle->BuidlBasicCar (basicCarParameters);

	// set this vehicle as the player
	manager->SetAsPlayer(heavyVehicle);
	

	// run the simulation loop
	for (int i = 0; i < 300; i ++) {
		// this function will advance the simulation by the timestep, on each call
		gNewton->UpdateOffLine (1.0f/60.f);

		std::cout << "running...\n ";
	}

	delete gNewton;

	return 0;
}

