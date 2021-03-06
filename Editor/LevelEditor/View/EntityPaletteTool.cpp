#include "Stdafx.h"
#include "EntityPaletteTool.h"
#include "LevelEditorWindow.h"
#include "..\Proxy\FolderProxy.h"
#include "..\IsilmeControl.h"
#include "..\ResourceHelper.h"

namespace LevelEditor
{
	namespace View
	{
		public ref class PaletteDataTemplateSelector : public System::Windows::Controls::DataTemplateSelector
		{
		public:
			PaletteDataTemplateSelector()
			{

			}
			virtual System::Windows::DataTemplate^ SelectTemplate(Object^ item, DependencyObject^ container) override
			{
				FrameworkElement^ element = (FrameworkElement^)container;

				if (element != nullptr && item != nullptr && dynamic_cast<Proxy::PaletteItemProxy^>(item) != nullptr)
					return EntityPaletteTool::itemTemplate;
				else
					return EntityPaletteTool::folderTemplate;

				return nullptr;
			}
		};

		EntityPaletteTool::EntityPaletteTool()
		{
			mPaletteTree = gcnew TreeView();
			mPaletteTree->MouseUp += gcnew System::Windows::Input::MouseButtonEventHandler(this, &EntityPaletteTool::OnItemSelected);
			AddChild(mPaletteTree);
		//	Command = LevelEditorWindow::SelectPaletteItem;

			Title = "Palette";

			Icon = ResourceHelper::GetPngSource("Palette.png");
		}

		EntityPaletteTool^ EntityPaletteTool::Instance::get()
		{
			if (mInstance == nullptr)
				mInstance = gcnew EntityPaletteTool();

		//	IsilmeHost::LoadGameData();

			return mInstance;
		}

		EntityPaletteTool::~EntityPaletteTool()
		{

		}

		void EntityPaletteTool::OnItemSelected( Object^ sender, MouseButtonEventArgs^ e )
		{
			mCurrentItem = SelectedItem;
			//RoutedCommand^ c = dynamic_cast<RoutedCommand^>(Command);
			//LevelEditorWindow::SelectPaletteItem->Execute(mPaletteTree->SelectedItem, LevelEditorWindow::Instance);
			//LevelEditorWindow::AddLayerCommand->Execute(nullptr, this);
		}

		//void EntityPaletteTool::CommandChanged( DependencyObject^ d, DependencyPropertyChangedEventArgs e )
		//{
		//	EntityPaletteTool^ tool = dynamic_cast<EntityPaletteTool^>(d);
		//	if (tool == nullptr)
		//		return;
		//	
		//	tool->HookUpCommand(( System::Windows::Input::ICommand^)e.OldValue,( System::Windows::Input::ICommand^)e.NewValue);
		//}

		//void EntityPaletteTool::HookUpCommand(  System::Windows::Input::ICommand^ oldCommand,  System::Windows::Input::ICommand^ newCommand )
		//{
		//	// If oldCommand is not null, then we need to remove the handlers.
		//	if (oldCommand != nullptr)
		//	{
		//		RemoveCommand(oldCommand, newCommand);
		//	}

		//	AddCommand(oldCommand, newCommand);

		//}

		//void EntityPaletteTool::AddCommand(  System::Windows::Input::ICommand^ oldCommand,  System::Windows::Input::ICommand^ newCommand )
		//{
		//	EventHandler^ handler = gcnew EventHandler(this, &EntityPaletteTool::CanExecuteChanged);
		//	canExecuteChangedHandler = handler;
		//	if (newCommand != nullptr)
		//	{
		//		newCommand->CanExecuteChanged += canExecuteChangedHandler;
		//	}
		//}

		//void EntityPaletteTool::RemoveCommand(  System::Windows::Input::ICommand^ oldCommand,  System::Windows::Input::ICommand^ newCommand )
		//{
		//	EventHandler^ handler = canExecuteChangedHandler;
		//	oldCommand->CanExecuteChanged -= handler;
		//}

		//void EntityPaletteTool::CanExecuteChanged( Object^ sender, EventArgs^ e )
		//{
		//	if (this->Command != nullptr)
		//	{
		//		RoutedCommand^ command = dynamic_cast<RoutedCommand^>(this->Command);

		//		// If a RoutedCommand.
		//		if (command != nullptr)
		//		{
		//			if (command->CanExecute(CommandParameter, CommandTarget))
		//			{
		//			//	this->IsEnabled = true;
		//			}
		//			else
		//			{
		//			//	this->sEnabled = false;
		//			}
		//		}
		//		// If a not RoutedCommand.
		//		else
		//		{
		//			if (Command->CanExecute(CommandParameter))
		//			{
		//			//	this.IsEnabled = true;
		//			}
		//			else
		//			{
		//			//	this.IsEnabled = false;
		//			}
		//		}
		//	}
		//}

		EntityPaletteProxy^ EntityPaletteTool::Palette::get()
		{
			return mPalette;
		}

		void EntityPaletteTool::Palette::set(EntityPaletteProxy^ value)
		{
			mPalette = value;
			FrameworkElementFactory^ stackElementFactory;
			FrameworkElementFactory^ iconEF;

			// ����������� ������
			System::Windows::Data::Binding^ binding = gcnew System::Windows::Data::Binding("Items");
			mPaletteTree->SetBinding(TreeView::ItemsSourceProperty, binding);
			mPaletteTree->DataContext = value;//->Layers[0];


			// ������ ��� �������� �������
			itemTemplate = gcnew HierarchicalDataTemplate(PaletteItemProxy::typeid);
			stackElementFactory = gcnew FrameworkElementFactory(StackPanel::typeid);
			stackElementFactory->SetValue(StackPanel::OrientationProperty, System::Windows::Controls::Orientation::Horizontal);

			iconEF = gcnew FrameworkElementFactory(System::Windows::Controls::Image::typeid);
			iconEF->SetValue(System::Windows::Controls::Image::SourceProperty, ResourceHelper::GetPngSource("PaletteItem.png"));

			FrameworkElementFactory^ itemElementFactory = gcnew FrameworkElementFactory(TextBlock::typeid);

			stackElementFactory->AppendChild(iconEF);
			stackElementFactory->AppendChild(itemElementFactory);
			itemElementFactory->SetBinding(TextBlock::TextProperty, gcnew System::Windows::Data::Binding("Name"));
			itemTemplate->VisualTree = stackElementFactory;
			
			

			// ������ ��������
			folderTemplate = gcnew HierarchicalDataTemplate(FolderProxy::typeid);

			stackElementFactory = gcnew FrameworkElementFactory(StackPanel::typeid);
			stackElementFactory->SetValue(StackPanel::OrientationProperty, System::Windows::Controls::Orientation::Horizontal);

			iconEF = gcnew FrameworkElementFactory(System::Windows::Controls::Image::typeid);
			iconEF->SetValue(System::Windows::Controls::Image::SourceProperty, ResourceHelper::GetPngSource("FolderClosed.png"));

			FrameworkElementFactory^ folderElementFactory = gcnew FrameworkElementFactory(TextBlock::typeid);
			folderElementFactory->SetBinding(TextBlock::TextProperty, gcnew System::Windows::Data::Binding("Name"));

			stackElementFactory->AppendChild(iconEF);
			stackElementFactory->AppendChild(folderElementFactory);

			folderTemplate->VisualTree = stackElementFactory;
			folderTemplate->ItemsSource = gcnew System::Windows::Data::Binding("Items");
			folderTemplate->ItemTemplateSelector = gcnew PaletteDataTemplateSelector();

			// ������������� ������
			mPaletteTree->ItemTemplateSelector = folderTemplate->ItemTemplateSelector;
		}

		//Object^ EntityPaletteTool::CommandParameter::get()
		//{
		//	return mPaletteTree->SelectedItem;
		//}

		//IInputElement^ EntityPaletteTool::CommandTarget::get()
		//{
		//	return this;
		//}

		//System::Windows::Input::ICommand^ EntityPaletteTool::Command::get()
		//	{
		//		return (System::Windows::Input::ICommand^)GetValue(CommandProperty);
		//	}
		//	void EntityPaletteTool::Command::set(System::Windows::Input::ICommand^ value) 
		//	{
		//		SetValue(CommandProperty, value);
		//	}

		PaletteItemProxy^ EntityPaletteTool::SelectedItem::get()
		{
			PaletteItemProxy^ item = dynamic_cast<PaletteItemProxy^>(mPaletteTree->SelectedItem);
			if (item != nullptr && item != mCurrentItem)
				mCurrentItem = item;
			return mCurrentItem;
		}
	}
}