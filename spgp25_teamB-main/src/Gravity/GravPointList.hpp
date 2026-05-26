/// @file GravPointList.hpp
/// @author Xein
/// @date 22/02/26

#pragma once
#include <Toast/Components/Component.hpp>
#include <Toast/Event/ListenerComponent.hpp>

class GravPoint;
class Player;
struct GravityBegin;
struct GravityEnd;

class GravPointList : public toast::Component {
public:
	REGISTER_TYPE(GravPointList);

	void Init() override;
	void Begin() override;
	void Tick() override;

	void Destroy() override { }

	void AddPoint();
	void RemovePoint(GravPoint* point);

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	void OnGravityBegin();
	void OnGravityEnd();

private:
	struct {
		event::ListenerComponent listener;
		std::list<GravPoint*> gravityPoints;
		GravPoint* nearestPoint;
		Player* player = nullptr;

		bool isGravityRunning = false;
	} m;
};
