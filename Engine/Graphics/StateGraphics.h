#ifndef ISILME_GRAPHICS_STATEGRAPHICS_H
#define ISILME_GRAPHICS_STATEGRAPHICS_H

#include "Definitions.h"
#include "SpecialGraphics.h"

namespace graphics
{
	template <class T>
	class StateGraphicsDef : public GraphicsDefinition
	{
	public:
		virtual GraphicsPtr Create();
		virtual void Parse(TiXmlElement* element);

		GraphicsPalette Palette;
		std::string		State;
	};

	/// ������� �� ����������
	template <class T>
	class StateGraphics : public SpecialGraphics<T>
	{
	public:
		StateGraphics(StateGraphicsDef<T>* def);
		virtual ~StateGraphics();

		/// ���������� ������� ��� ���������
		void	SetState(std::string state);

		GraphicsPtr	GetState();

		GraphicsPtr GetState(std::string name);

		/// ���������� ������� ���������
		virtual void Render(float x, float y, float angle);

		virtual void Update(float elapsedTime);

		virtual void SelectState();
	protected:
	private:
		GraphicsMap mGraphics;
		GraphicsPtr mCurrentGraphics;
	};


	/// IMPLEMENTATION

	template<class T>
	StateGraphics<T>::StateGraphics(StateGraphicsDef<T>* def)
	{
		// ������� ������� ������� ���������
		for (GraphicsPalette::iterator it = def->Palette.begin(); it != def->Palette.end(); ++it)
		{
			GraphicsPtr g = it->second->Create();
			mGraphics[it->first] = g;
		}

		// ������������� ������� �������
		SetState(def->State);
	}
	
	template<class T>
	StateGraphics<T>::~StateGraphics()
	{
		mGraphics.clear();
		mCurrentGraphics.reset();
	}

	template<class T>
	GraphicsPtr StateGraphics<T>::GetState(std::string name)
	{
		GraphicsMap::iterator it = mGraphics.find(name);
		return it->second;
	}

	template<class T>
	void StateGraphics<T>::SetState(std::string state)
	{
		GraphicsMap::iterator it = mGraphics.find(state);
		if (it != mGraphics.end())
		{
			mCurrentGraphics = it->second;
		}
	}

	template<class T>
	void StateGraphics<T>::Render(float x, float y, float angle)
	{
		mCurrentGraphics->Render(x, y, angle);
		Graphics::Render(x,y,angle);
	}

	template<class T>
	void StateGraphics<T>::Update(float elapsedTime)
	{
		SelectState();
		mCurrentGraphics->Update(elapsedTime);
		Graphics::Update(elapsedTime);
	}

	template<class T>
	void StateGraphics<T>::SelectState()
	{
	}

	template<class T>
	GraphicsPtr StateGraphics<T>::GetState()
	{
		return mCurrentGraphics;
	}

	template<class T>
	GraphicsPtr StateGraphicsDef<T>::Create()
	{
		GraphicsPtr g(new StateGraphics<T>(this));

		return g;
	}

	template<class T>
	void StateGraphicsDef<T>::Parse(TiXmlElement* element)
	{
		TiXmlElement* stateElement = element->FirstChildElement("State");
		char* state = const_cast<char*>(element->Attribute("State"));

		while (stateElement)
		{
			char* name = const_cast<char*>(stateElement->Attribute("Name"));
			if (name == 0) name = "Default";
			
			if (state == 0)
				state = name;

			TiXmlElement* graphicsElement = stateElement->FirstChildElement();

			GraphicsDefPtr def = FactoryManager::GetSingleton()->LoadGraphics(graphicsElement);
			Palette[name] = def;

			stateElement = stateElement->NextSiblingElement("State");
		}

		State = state;
	}
};

#endif