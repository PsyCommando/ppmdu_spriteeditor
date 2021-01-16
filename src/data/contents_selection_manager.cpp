#include "contents_selection_manager.hpp"
#include <src/data/content_manager.hpp>

ContentsSelectionManager::ContentsSelectionManager()
{
    resetModel();
}

ContentsSelectionManager &ContentsSelectionManager::Instance()
{
    static ContentsSelectionManager s_instance;
    return s_instance;
}

QItemSelectionModel *ContentsSelectionManager::SelectionModel()
{
    return Instance().selectionModel();
}

QItemSelectionModel *ContentsSelectionManager::selectionModel()
{
    return m_selection.data();
}

void ContentsSelectionManager::resetModel()
{
    m_selection.reset(new QItemSelectionModel(&ContentManager::Instance()));
}
