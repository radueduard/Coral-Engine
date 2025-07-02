using System;
using Coral.ECS;
using Coral.ECS.components;

namespace Coral
{
    public class CameraController : Behaviour
    {

        public override void Start()
        {
            Console.WriteLine("CameraController::Start()");
        }

        public override void Update()
        {
            Console.WriteLine("CameraController::Update()");
        }

        public override void FixedUpdate()
        {
            Console.WriteLine("CameraController::FixedUpdate()");
        }

        public override void LateUpdate()
        {
            Console.WriteLine("CameraController::LateUpdate()");
        }
    }
}