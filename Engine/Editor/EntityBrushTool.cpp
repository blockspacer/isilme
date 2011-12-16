#include "IsilmePCH.h"
#include "EntityBrushTool.h"
#include "AddEntityCommand.h"
#include "CommandManager.h"
#include "Layer.h"
#include "Level.h"
#include "Entity.h"

namespace editor
{
	EntityBrushTool::EntityBrushTool()
	{
	}

	EntityBrushTool::~EntityBrushTool()
	{
	}

	void EntityBrushTool::OnMouseMove(LayerPtr layer, float x, float y, int button)
	{
	}

	void EntityBrushTool::OnMouseDown(LayerPtr layer, float x, float y, int button)
	{
	}

	void EntityBrushTool::OnMouseUp(LayerPtr layer, float x, float y, int button)
	{
		EntityPtr entity = FactoryManager::GetSingleton()->CreateEntity(mEntityTypeID, "");

		CommandPtr coomand(new AddEntityCommand(layer, entity));
		GetCommandManager()->Execute(coomand);

		Vector2 mouse(x,y);
		mouse *= 1.0f / 64.0f;
		mouse.x += layer->GetLevel()->GetActiveCamera()->x;
		mouse.y += layer->GetLevel()->GetActiveCamera()->y;
		entity->SetPosition(mouse.x, mouse.y);
		entity->SetAngle(0);
	}

	void EntityBrushTool::SetEntityTypeID(std::string id)
	{
		mEntityTypeID = id;
	}
};