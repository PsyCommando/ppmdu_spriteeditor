#ifndef CONTENTSSELECTIONMANAGER_HPP
#define CONTENTSSELECTIONMANAGER_HPP
#include <QItemSelectionModel>

/*
 * Singleton shared selection model for the main tree view selection
*/
class ContentsSelectionManager
{
    ContentsSelectionManager();
public:
    static ContentsSelectionManager& Instance();
    static QItemSelectionModel * SelectionModel();

    QItemSelectionModel * selectionModel();
    const QItemSelectionModel * selectionModel()const {return const_cast<ContentsSelectionManager*>(this)->SelectionModel();}

    void resetModel();

private:
    QScopedPointer<QItemSelectionModel> m_selection;
};

#endif // CONTENTSSELECTIONMANAGER_HPP
