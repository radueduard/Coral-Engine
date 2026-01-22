//
// Created by radue on 1/17/2026.
//

#pragma once
#include <boost/uuid/string_generator.hpp>

#include "ecs/components/renderTarget.h"

#include "graphics/objects/mesh.h"

namespace Coral::PlaneMeshes {
	inline std::unique_ptr<Graphics::Mesh> WingLeft() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { -1.0f, 0.1f, 0.5f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Wing Left")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> WingRight() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { 1.0f, 0.1f, 0.5f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Wing Right")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.Build();
	}


	inline std::unique_ptr<Graphics::Mesh> Rudder() {
		Math::Vector3f p0t = { 0.f, 0.0f, 0.f };
		Math::Vector3f p1t = { 0.f, 0.0f, 0.f };

		Math::Vector3f p0b = { 0.1f, -.1f, 1.0f };
		Math::Vector3f p1b = { -0.1f, -.1f, 1.0f };

		Math::Vector3f p2 = { 0.f, 1.f, 0.f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Rudder")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> FlapLeft() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { -1.0f, 0.1f, 0.1f };
		Math::Vector3f p3 = { -1.0f, 0.0f, 0.15f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Flap Left")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p3 })

			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p3 })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p3 })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.AddIndex(12).AddIndex(13).AddIndex(14)
			.AddIndex(15).AddIndex(16).AddIndex(17)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> FlapRight() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { 1.0f, 0.1f, 0.1f };
		Math::Vector3f p3 = { 1.0f, 0.0f, 0.15f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Flap Right")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p3 })

			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p3 })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p3 })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.AddIndex(12).AddIndex(13).AddIndex(14)
			.AddIndex(15).AddIndex(16).AddIndex(17)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> ElevatorLeft() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { -1.0f, 0.1f, 0.1f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Elevator Left")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> ElevatorRight() {
		Math::Vector3f p0t = { 0.f, 0.1f, -0.1f };
		Math::Vector3f p1t = { 0.f, 0.1f, 0.1f };

		Math::Vector3f p0b = { 0.f, 0.0f, -0.1f };
		Math::Vector3f p1b = { 0.f, 0.0f, 0.1f };

		Math::Vector3f p2 = { 1.0f, 0.1f, -0.1f };

		return  Graphics::Mesh::Builder(boost::uuids::random_generator()())
			.Name("Elevator Right")
			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p0t })
			.AddVertex({ .position = p0b })
			.AddVertex({ .position = p2 })

			.AddVertex({ .position = p1t })
			.AddVertex({ .position = p1b })
			.AddVertex({ .position = p2 })

			.AddIndex(0).AddIndex(1).AddIndex(2)
			.AddIndex(3).AddIndex(4).AddIndex(5)
			.AddIndex(6).AddIndex(7).AddIndex(8)
			.AddIndex(9).AddIndex(10).AddIndex(11)
			.Build();
	}

	inline std::unique_ptr<Graphics::Mesh> Back() {
		auto cone = Graphics::Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000007"));
		cone
			.Name("ConeWithOffsetTip");

		Math::Vector3f apexPosition = { 1.0f, 1.0f, 0.0f };
		Math::Vector3f apexNormal = { 0.0f, 1.0f, 0.0f };
		Math::Vector4f apexTangent = { 1.0f, 0.0f, 0.0f, 1.0f };
		Math::Vector2f apexTexCoord = { 0.5f, 0.0f };
		cone.AddVertex({ apexPosition, apexNormal, apexTangent, apexTexCoord });

		for (int i = 0; i <= 16; i++) {
			const float theta = static_cast<float>(i) * 2.0f * glm::pi<float>() / static_cast<float>(16);
			Math::Vector3f basePosition = { cos(theta), -1.0f, sin(theta) };
			Math::Vector3f baseNormal = Math::Vector3f { cos(theta), 0.0f, sin(theta) }.Normalized();
			Math::Vector3f tangent = { -sin(theta), 0.0f, cos(theta) };
			Math::Vector2f baseTexCoord = {
				(static_cast<float>(i) / static_cast<float>(16)),
				1.0f
			};
			Math::Vector4f tangent4 = Math::Vector4(tangent, 1.f);
			cone.AddVertex({ basePosition, baseNormal, tangent4, baseTexCoord });
		}

		for (int i = 1; i <= 16; i++) {
			cone.AddIndex(0).AddIndex(i).AddIndex(i + 1);
		}

		const int baseCenterIndex = 16 + 2;
		cone.AddVertex({ {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f} });

		for (int i = 1; i <= 16; i++) {
			const int currentBaseIndex = i;
			const int nextBaseIndex = (i % 16) + 1;
			cone.AddIndex(baseCenterIndex).AddIndex(nextBaseIndex).AddIndex(currentBaseIndex);
		}

		return cone.Build();
	}

	inline std::unique_ptr<ECS::Entity> Plane() {
		auto leftWing = PlaneMeshes::WingLeft();
    	auto rightWing = PlaneMeshes::WingRight();
    	auto tail = PlaneMeshes::Back();
    	auto rudder = PlaneMeshes::Rudder();
    	auto leftElevator = PlaneMeshes::ElevatorLeft();
    	auto rightElevator = PlaneMeshes::ElevatorRight();
    	auto flapLeft = PlaneMeshes::FlapLeft();
		auto flapRight = PlaneMeshes::FlapRight();

    	auto planeEntity = std::make_unique<ECS::Entity>("Plane");
    	auto leftWingEntity = std::make_unique<ECS::Entity>("Left Wing");
    	auto leftWingTipEntity = std::make_unique<ECS::Entity>("Left Wing Tip");
		auto rightWingEntity = std::make_unique<ECS::Entity>("Right Wing Tip");
		auto rightWingTipEntity = std::make_unique<ECS::Entity>("Right Wing");
    	auto bodyEntity = std::make_unique<ECS::Entity>("Body");
    	auto cockpitEntry = std::make_unique<ECS::Entity>("Cockpit");
    	auto tailEntity = std::make_unique<ECS::Entity>("Tail");
    	auto rudderEntity = std::make_unique<ECS::Entity>("Rudder");
    	auto leftElevatorEntity = std::make_unique<ECS::Entity>("Left Elevator");
    	auto rightElevatorEntity = std::make_unique<ECS::Entity>("Right Elevator");
    	auto leftFlapEntity = std::make_unique<ECS::Entity>("Left Flap");
    	auto rightFlapEntity = std::make_unique<ECS::Entity>("Right Flap");


    	leftWingEntity->Get<ECS::Transform>().position = Math::Vector3f { -0.45f, 0.f, -0.5f };
    	leftWingEntity->Get<ECS::Transform>().scale = Math::Vector3f { 3.5f, 1.75f, 3.5f };

    	leftWingTipEntity->Get<ECS::Transform>().position = Math::Vector3f { -2.0f, 0.f, 0.43f };
    	leftWingTipEntity->Get<ECS::Transform>().scale = Math::Vector3f { 3.5f, 1.75f, 3.5f };

    	rightWingEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.45f, 0.f, -0.5f };
    	rightWingEntity->Get<ECS::Transform>().scale = Math::Vector3f { 3.5f, 1.75f, 3.5f };

    	rightWingTipEntity->Get<ECS::Transform>().position = Math::Vector3f { 2.0f, 0.f, 0.43f };
		rightWingTipEntity->Get<ECS::Transform>().scale = Math::Vector3f { 3.5f, 1.75f, 3.5f };

    	bodyEntity->Get<ECS::Transform>().scale = Math::Vector3f { 0.5f, 3.5f, 0.5f };
    	bodyEntity->Get<ECS::Transform>().rotation = Math::Vector3f { 90.f, 0.f, 0.f };

    	cockpitEntry->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, -4.f };
    	cockpitEntry->Get<ECS::Transform>().rotation = Math::Vector3f { -90.f, 0.f, 0.f };
		cockpitEntry->Get<ECS::Transform>().scale = Math::Vector3f { 0.5f, 0.5f, 0.5f };

		tailEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 4.f };
    	tailEntity->Get<ECS::Transform>().rotation = Math::Vector3f { 90.f, 0.f, 90.f };
    	tailEntity->Get<ECS::Transform>().scale = Math::Vector3f { 0.5f };

    	rudderEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, .5f, 4.5f };
    	rudderEntity->Get<ECS::Transform>().rotation = Math::Vector3f { 180.f, 0.f, 180.f };
    	rudderEntity->Get<ECS::Transform>().scale = Math::Vector3f { 1.f, 1.5f, 1.f };

    	leftElevatorEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 3.5f };
		leftElevatorEntity->Get<ECS::Transform>().scale = Math::Vector3f { 2.f, 1.f, 4.f };

    	rightElevatorEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 3.5f };
    	rightElevatorEntity->Get<ECS::Transform>().scale = Math::Vector3f { 2.f, 1.f, -4.f };

    	leftFlapEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 0.1f };
    	leftFlapEntity->Get<ECS::Transform>().scale = Math::Vector3f { 2.f, 1.f, 4.f };

    	rightFlapEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 0.1f };
    	rightFlapEntity->Get<ECS::Transform>().rotation = Math::Vector3f { 0.f, 0.f, 180.f };
    	rightFlapEntity->Get<ECS::Transform>().scale = Math::Vector3f { -2.f, -1.f, 4.f };

    	leftWingEntity->Add<ECS::RenderTarget>().Add(leftWing.get());
    	leftWingTipEntity->Add<ECS::RenderTarget>().Add(leftWing.get());
    	rightWingEntity->Add<ECS::RenderTarget>().Add(rightWing.get());
    	rightWingTipEntity->Add<ECS::RenderTarget>().Add(rightWing.get());
    	bodyEntity->Add<ECS::RenderTarget>().Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000003")));
    	cockpitEntry->Add<ECS::RenderTarget>().Add(Context::AssetManager().GetMesh(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000005")));
		tailEntity->Add<ECS::RenderTarget>().Add(tail.get());
    	rudderEntity->Add<ECS::RenderTarget>().Add(rudder.get());
    	leftElevatorEntity->Add<ECS::RenderTarget>().Add(leftElevator.get());
    	rightElevatorEntity->Add<ECS::RenderTarget>().Add(rightElevator.get());
    	leftFlapEntity->Add<ECS::RenderTarget>().Add(flapLeft.get());
    	rightFlapEntity->Add<ECS::RenderTarget>().Add(flapRight.get());

    	planeEntity->AddChild(std::move(leftWingEntity));
    	planeEntity->AddChild(std::move(leftWingTipEntity));
		planeEntity->AddChild(std::move(rightWingEntity));
    	planeEntity->AddChild(std::move(rightWingTipEntity));
    	planeEntity->AddChild(std::move(bodyEntity));
    	planeEntity->AddChild(std::move(cockpitEntry));
    	planeEntity->AddChild(std::move(tailEntity));
    	planeEntity->AddChild(std::move(rudderEntity));
    	planeEntity->AddChild(std::move(leftElevatorEntity));
    	planeEntity->AddChild(std::move(rightElevatorEntity));
    	planeEntity->AddChild(std::move(leftFlapEntity));
    	planeEntity->AddChild(std::move(rightFlapEntity));

		planeEntity->Get<ECS::Transform>().scale = Math::Vector3f { 0.1f };
		planeEntity->Get<ECS::Transform>().rotation = Math::Vector3f { 90.f, 0.f, 45.f };
		planeEntity->Get<ECS::Transform>().position = Math::Vector3f { 0.f, 0.f, 25.f };

		Context::AssetManager().AddMesh(std::move(leftWing));
		Context::AssetManager().AddMesh(std::move(rightWing));
		Context::AssetManager().AddMesh(std::move(tail));
		Context::AssetManager().AddMesh(std::move(rudder));
		Context::AssetManager().AddMesh(std::move(leftElevator));
		Context::AssetManager().AddMesh(std::move(rightElevator));
		Context::AssetManager().AddMesh(std::move(flapLeft));
		Context::AssetManager().AddMesh(std::move(flapRight));

		return planeEntity;
	}

}
