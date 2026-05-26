#include "GravPoint.hpp"

void GravPoint::Init() {
	mesh = children.AddRequired<toast::MeshRendererComponent>();
	mesh->SetMaterial("WHITEBOXING/MAGNENT.mat");
}

void GravPoint::Using() {
	mesh->SetMaterial("WHITEBOXING/MAGNENT_USING.mat");
}

void GravPoint::Disable() {
	mesh->SetMaterial("WHITEBOXING/MAGNET_DISABLE.mat");
}

void GravPoint::Enable() {
	mesh->SetMaterial("WHITEBOXING/MAGNET_ENABLE.mat");
}
