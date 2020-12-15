#include "animtation_table_layout.hpp"
#include <src/ui/errorhelper.hpp>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <src/data/sprite/models/animtable_model.hpp>
#include <src/data/sprite/animtable.hpp>
#include <src/data/sprite/sprite.hpp>

const QString XML_ATTR_SPRITE_UTIL_VERSION = "sprite_editor_version";
const QString XML_NODE_ANIM_TABLE_LAYOUT = "AnimTableLayout";
const QString XML_ATTR_SPRITE_TYPE = "sprite_type";
const QString XML_NODE_SLOT = "Slot";
const QString XML_ATTR_NAME = "name";

void ImportAnimTableLayout(AnimTableModel *dst, const QString &filepath)
{
    QFile               infile(filepath);
    QXmlStreamReader    reader(&infile);
    if(!infile.open(QFile::OpenModeFlag::ReadOnly) || infile.error() != QFile::FileError::NoError)
    {
        ErrorHelper::getInstance().sendErrorMessage(QString("ImportAnimTableLayout(): Couldn't open file \"%1\"").arg(filepath));
        return;
    }

    QVector<QString> slotsnames;
    while(!reader.atEnd())
    {
        QXmlStreamReader::TokenType tt = reader.readNext();
        if(tt == QXmlStreamReader::Comment)
            continue;
        if(tt == QXmlStreamReader::StartElement)
        {
            if(reader.name() == XML_NODE_ANIM_TABLE_LAYOUT)
            {
                QXmlStreamAttributes attr = reader.attributes();
                if(attr.hasAttribute(XML_ATTR_SPRITE_TYPE))
                {
                    QString attsprty = attr.value(XML_ATTR_SPRITE_TYPE).toString();
                    qDebug() << QString("Imported sprite type %1").arg(attsprty);
                }
            }
            if(reader.name() == XML_NODE_SLOT)
            {
                QXmlStreamAttributes attr = reader.attributes();
                if(attr.hasAttribute(XML_ATTR_NAME))
                    slotsnames.push_back(attr.value(XML_ATTR_NAME).toString());
                else
                    slotsnames.push_back(QString()); //We still count empty slot as just having empty names
            }
        }
    }

    if(reader.hasError())
    {
        //error
        ErrorHelper::getInstance().sendErrorMessage(QString("ImportAnimTableLayout(): Error : %1").arg(reader.errorString()));
        return;
    }

    //Fill the model
    int difference = slotsnames.size() - dst->rowCount();
    if(difference > 0)
    {
        //Add missing rows
        dst->insertRows(dst->rowCount(), abs(difference));
    }
    else if(difference < 0)
    {
        //Remove extra rows
        dst->removeRows(abs(difference) - 1, abs(difference));
    }

    //Set names for all slots
    for(int i = 0; i < slotsnames.size(); ++i)
    {
        dst->setData(dst->index(i,
                               0,
                               QModelIndex()).siblingAtColumn(static_cast<int>(AnimGroup::eColumns::GroupName)),
                    slotsnames[i],
                    Qt::EditRole);
    }
}

void ExportAnimTableLayout(const AnimTableModel *src, const QString &filepath)
{
    QFile               outf(filepath);
    QXmlStreamWriter    xml(&outf);
    if(!outf.open(QFile::OpenModeFlag::WriteOnly) && outf.error() != QFile::FileError::NoError)
    {
        ErrorHelper::getInstance().sendErrorMessage(QString("ExportAnimTableLayout(): Couldn't create file \"%1\"").arg(filepath));
        return;
    }

    const Sprite * spr = src->getOwnerSprite();
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement(XML_NODE_ANIM_TABLE_LAYOUT);
    xml.writeAttribute(XML_ATTR_SPRITE_UTIL_VERSION, QString("%1").arg(GIT_MAJORMINOR_VERSION));
    xml.writeAttribute(XML_ATTR_SPRITE_TYPE, QString("%1").arg(static_cast<int>(spr->type())));
    for(int i = 0; i < src->rowCount(); ++i)
    {
        xml.writeEmptyElement(XML_NODE_SLOT);
        QModelIndex curslot = src->index(i, static_cast<int>(AnimGroup::eColumns::GroupName), QModelIndex());
        QString name = src->data(curslot, Qt::EditRole).toString();
        xml.writeAttribute(XML_ATTR_NAME, name);
    }
    xml.writeEndElement();
    xml.writeEndDocument();
}
