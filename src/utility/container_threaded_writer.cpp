#include "container_threaded_writer.hpp"
#include <QDataStream>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/sprite_container.hpp>
#include <src/ppmdu/fmts/packfile.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ui/errorhelper.hpp>


//=================================================================================================
//  ThreadedWriter
//=================================================================================================
ContainerThreadedWriter::ContainerThreadedWriter(QSaveFile *sfile, BaseContainer *cnt)
    :QObject(nullptr),savefile(sfile), container(cnt), bywritten(0)
{

}

ContainerThreadedWriter::~ContainerThreadedWriter()
{
    qDebug("ThreadedWriter::~ThreadedWriter()\n");
}

void ContainerThreadedWriter::WritePack()
{
    try
    {
        _WritePack();
    }
    catch(const std::exception & e)
    {
        ErrorHelper::getInstance().sendErrorMessage(e.what());
    }
}

void ContainerThreadedWriter::WriteSprite()
{
    try
    {
        _WriteSprite();
    }
    catch(const std::exception & e)
    {
        ErrorHelper::getInstance().sendErrorMessage(e.what());
    }
}

void ContainerThreadedWriter::_WriteSprite()
{
    Q_ASSERT(container && container->GetNbDataRows() );
    QDataStream     outstr(savefile.data());
    Sprite * node = static_cast<Sprite*>(container->nodeChild(0));
    bywritten   = 0;

    //Setup the functions
    op1 = QtConcurrent::run( [&]()
    {
        try
        {
            node->DumpSpriteToStream(outstr);
            savefile->commit();
        }
        catch(const std::exception & e )
        {
            ErrorHelper::getInstance().sendErrorMessage(e.what());
        }
    });
    op2 = QtConcurrent::run( [](){} ); //dummy op

    QFutureSynchronizer<void> futsync;
    futsync.addFuture(op1);
    futsync.waitForFinished();


    op2.waitForFinished();
    emit finished(bywritten);
    emit finished();
    qDebug("ThreadedWriter::WritePack(): WriteSprite!\n");
}

void ContainerThreadedWriter::_WritePack()
{
    Q_ASSERT(container && container->nodeHasChildren());
    bywritten = 0;

    SpriteContainer * sprcnt = static_cast<SpriteContainer*>(container);

    //Commit all sprites that need it!
    op1 = QtConcurrent::map( sprcnt->begin(),
                               sprcnt->end(),
                               [&](Sprite * curspr)
    {
        try
        {
            if(curspr->hasUnsavedChanges())
                curspr->CommitSpriteData();
        }
        catch(const std::exception & e)
        {
            ErrorHelper::getInstance().sendErrorMessage(e.what());
        }
    });
    op2 = QtConcurrent::run([&]()
    {
        op1.waitForFinished();
        //Fill up the packfilewriter
        fmt::PackFileWriter writer;
        for( Sprite * spr : *sprcnt )
            writer.AppendSubFile( spr->getRawData().begin(), spr->getRawData().end() );

        //Build the packfile
        QDataStream outstr(savefile.data());
        QByteArray  out;
        writer.Write(std::back_inserter(out));

        //Write it to file!
        bywritten = outstr.writeRawData( out.data(), out.size() );
        savefile->commit();
    });

    //Wait for it to finish
    QFutureSynchronizer<void> futsync;
    futsync.addFuture(op1);
    futsync.addFuture(op2);
    futsync.waitForFinished();

    emit finished(bywritten);
    emit finished();
    qDebug("ThreadedWriter::WritePack(): Finished!\n");
}
