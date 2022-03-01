#pragma once

#include "hzpch.h"
#include "Hazel/Core.h"



namespace Hazel
{
	//Currently our events in hazel are blocking which means when an event occurs it must be dealt with then and there
	//for the future we will make an event bus to buffer events and process them during the event part

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, 
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory                                                                                                 //if we want to filter events
	{
		None = 0,
		EventCategoryApplication          = BIT(0), 
		EventCategoryInput 				  = BIT(1),
		EventCategoryKeyboard 			  = BIT(2),
		EventCategoryMouse 				  = BIT(3),
		EventCategoryMouseButton		  = BIT(4)
	};
//we use these macros so that we dont need to implment this code in every event subclass
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class HAZEL_API Event
	{
	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;                                   //only to be used for debug builds
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	class EventDispatcher
	{
	private:
		Event& m_Event;
	public:
		EventDispatcher(Event& event)                           //creates an intance of the event we just recieved
			: m_Event(event)
		{

		}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}