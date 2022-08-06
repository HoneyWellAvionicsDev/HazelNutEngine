#pragma once

#include "Entity.h"
#include "Hazel/Renderer/EditorCamera.h"




namespace Hazel
{
	namespace Utils
	{
		static const char* ScriptTypeToString(ScriptType type)
		{
			switch (type)
			{
				case ScriptType::None:				return "None";
				case ScriptType::CameraController:  return "CameraController";
				case ScriptType::Test:			    return "Test";
			}
			HZ_CORE_ASSERT(false, "Unknown type");
			return nullptr;
		}

		static ScriptType ScriptTypeFromString(const std::string& type)
		{
			if (type == "None")				return ScriptType::None;
			if (type == "CameraController") return ScriptType::CameraController;
			if (type == "Test")				return ScriptType::Test;

			HZ_CORE_ASSERT(false, "Unknown type");
			return ScriptType::None;
		}
	}

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity(){}

		template<typename T>
		T& GetComponent()
		{
			HZ_CORE_ASSERT(m_Entity.HasComponent<T>());
			return m_Entity.GetComponent<T>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep ts) {}
		Entity m_Entity;
		friend class Scene;
	private:
	};

    class CameraController : public ScriptableEntity
    {
    public:
		CameraController() = default;

        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(Timestep ts) override;
    };

	class Test : public ScriptableEntity
	{
	public:
		Test() = default;

		void OnCreate() override;
		void OnDestroy() override;
		void OnUpdate(Timestep ts) override;
	};
}

