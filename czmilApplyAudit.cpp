
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "czmilApplyAudit.hpp"


double settings_version = 2.0;


czmilApplyAudit::czmilApplyAudit (QWidget *parent)
  : QWizard (parent)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/czmilApplyAudit.png"));


  envin ();


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);

  setPage (0, new startPage (this));
  setPage (1, optionPg = new optionPage (this, &options));
  setPage (2, new fileInputPage (this, &inputFiles, options.input_dir));
  setPage (3, new runPage (this, &progress, &options));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start audit application"));
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



czmilApplyAudit::~czmilApplyAudit ()
{
}



void czmilApplyAudit::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);

  switch (id)
    {
    case 0:
      input_files.clear ();
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:

      button (QWizard::CustomButton1)->setEnabled (true);

      options.tolerance = field ("tolerance").toFloat ();
      options.return_check = field ("returnCheck").toBool ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      envout ();


      break;
    }
}



void czmilApplyAudit::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



//  This is where the fun stuff happens.

void 
czmilApplyAudit::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  Get the files from the QTextEdit box on the fileInputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList isort_files;

  isort_files.clear ();

  do
    {
      isort_files << inputCursor.block ().text ();
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  isort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (int32_t i = 0 ; i < isort_files.size () ; i++)
    {
      name = isort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  input_file_count = input_files.size ();


  //  Main processing loop

  CZMIL_CPF_Header                cpf_header;
  CZMIL_CPF_Data                  cpf;
  CZMIL_CAF_Header                caf_header;
  CZMIL_CAF_Data                  caf;
  int32_t                         cpf_hnd, caf_hnd, percent = 0, old_percent = -1, ret_count = 0, apply_count = 0;
  char                            string[1024];


  progress.obar->setRange (0, input_file_count * 100);


  //  Loop through each input file.

  for (int32_t i = 0 ; i < input_file_count ; i++)
    {
      ret_count = 0;
      apply_count = 0;

      name = input_files.at (i);

      strcpy (string, name.toLatin1 ());


      QString status;
      status.sprintf ("Processing file %d of %d : ", i + 1, input_file_count);
      status += QFileInfo (name).fileName ();

      QListWidgetItem *stat = new QListWidgetItem (status);

      progress.list->addItem (stat);
      progress.list->setCurrentItem (stat);
      progress.list->scrollToItem (stat);


      //  Try to open the input file.

      if ((caf_hnd = czmil_open_caf_file (string, &caf_header)) != CZMIL_SUCCESS)
        {
          QMessageBox::critical (this, tr ("czmilApplyAudit Open input file"), tr ("The file ") + name + 
                                 tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                 QString (czmil_strerror ()));
        }
      else
        {
          //  Try to open the output file.

          QString cpfName = name;
          cpfName.replace (".caf", ".cpf");
          char cpf_name[1024];
          strcpy (cpf_name, cpfName.toLatin1 ());

          if ((cpf_hnd = czmil_open_cpf_file (cpf_name, &cpf_header, CZMIL_UPDATE)) < 0)
            {
              QMessageBox::critical (this, tr ("czmilApplyAudit Create output file"), tr ("The file ") + cpfName + 
                                     tr (" could not be created.") + tr ("  The error message returned was:\n\n") +
                                     QString (czmil_strerror ()));
            }
          else
            {
              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (name).fileName ());

              progress.fbar->setRange (0, 100);


              //  Zero the counters.

              int32_t prev_rec = -1;


              //  Loop through the entire CAF file reading each record.

              for (uint32_t rec = 0 ; rec < caf_header.number_of_records ; rec++)
                {
                  if (czmil_read_caf_record (caf_hnd, &caf) != CZMIL_SUCCESS)
                    {
                      QMessageBox::critical (this, tr ("czmilApplyAudit Read CAF record"), tr ("Record %1 ").arg (rec) + tr ("in file ") + name + 
                                             tr (" could not be read.") + tr ("  The error message returned was:\n\n") +
                                             QString (czmil_strerror ()));
                      exit (-1);
                    }


                  //  If we changed record numbers update the last CPF record and read the new one.

                  if (prev_rec != caf.shot_id)
                    {
                      //  Update the previous record.

                      if (prev_rec >= 0)
                        {
                          if (czmil_update_cpf_return_status (cpf_hnd, prev_rec, &cpf) != CZMIL_SUCCESS)
                            {
                              QMessageBox::critical (this, tr ("czmilApplyAudit Update CPF record"), tr ("Record %1 ").arg (prev_rec) + tr ("in file ") + name + 
                                                     tr (" could not be updated.") + tr ("  The error message returned was:\n\n") +
                                                     QString (czmil_strerror ()));
                              exit (-1);
                            }
                        }

                          
                      //  Read the new CPF record.

                      if (czmil_read_cpf_record (cpf_hnd, caf.shot_id, &cpf) != CZMIL_SUCCESS)
                        {
                          QMessageBox::critical (this, tr ("czmilApplyAudit Read CPF record"), tr ("Record %1 ").arg (caf.shot_id) + tr ("in file ") + name + 
                                                 tr (" could not be read.") + tr ("  The error message returned was:\n\n") +
                                                 QString (czmil_strerror ()));
                          exit (-1);
                        }
                    }


                  //  Find the nearest interest point on this channel's waveform

                  float min_diff = 9999999.0, diff = -1.0;
                  int32_t ret = -1;

                  for (int32_t k = 0 ; k < cpf.returns[caf.channel_number] ; k++)
                    {
                      //  Check for the interest point being within the set tolerance.

                      diff = fabsf (caf.interest_point - cpf.channel[caf.channel_number][k].interest_point);
                      if (diff <= options.tolerance)
                        {
                          //  Make sure that the point was not filter invalidated during reprocessing

                          if (!(cpf.channel[caf.channel_number][k].status & CZMIL_RETURN_FILTER_INVAL))
                            {
                              //  Make sure that the Optech waveform processing mode has not changed

                              if (caf.optech_classification == cpf.optech_classification[caf.channel_number])
                                {
                                  if (diff < min_diff)
                                    {
                                      ret = k;
                                      min_diff = diff;
                                    }
                                }
                            }
                        }


                      //  If the difference has exceeded the minimum difference we can break out.  You can't break on the first 
                      //  point that falls within your tolerance because sometimes (believe it or not) there is more than one
                      //  point within your tolerance and you may end up invalidating the first one twice.  Once the difference
                      //  exceeds the min_diff that you set you have found the nearest point to the audit point that is within
                      //  tolerance.

                      if (diff > min_diff) break;
                    }


                  //  Perform the return number check if requested.

                  if (options.return_check && ret < 0)
                    {
                      //  Check to see if the number of returns is the same as before reprocessing.

                      if (cpf.returns[caf.channel_number] == caf.number_of_returns)
                        {
                          //  Make sure that the point was not filter invalidated during reprocessing

                          if (!(cpf.channel[caf.channel_number][caf.return_number].status & CZMIL_RETURN_FILTER_INVAL))
                            {
                              ret = caf.return_number;
                              ret_count++;
                            }
                        }
                    }


                  //  Now, if we've found the nearest and it meets our criteria, invalidate the point.

                  if (ret >= 0)
                    {
                      apply_count++;
                      cpf.channel[caf.channel_number][ret].status = CZMIL_RETURN_MANUALLY_INVAL;
                    }


                  //  We don't want to eat up all of the free time on the system with a progress bar.

                  percent = (int32_t) (((float) rec / (float) caf_header.number_of_records) * 100.0);
                  if (percent - old_percent >= 10 || old_percent > percent)
                    {
                      progress.fbar->setValue (percent);
                      old_percent = percent;

                      qApp->processEvents ();
                    }

                  prev_rec = caf.shot_id;
                }


              //  Update the last record.

              if (prev_rec >= 0)
                {
                  if (czmil_update_cpf_return_status (cpf_hnd, prev_rec, &cpf) != CZMIL_SUCCESS)
                    {
                      QMessageBox::critical (this, tr ("czmilApplyAudit Update CPF record"), tr ("Record %d ").arg (prev_rec) + tr ("in file ") + name + 
                                             tr (" could not be updated.") + tr ("  The error message returned was:\n\n") +
                                             QString (czmil_strerror ()));
                      exit (-1);
                    }
              
                  czmil_close_cpf_file (cpf_hnd);
                }


              progress.list->addItem (" ");
              progress.list->addItem (tr ("File : ") + cpfName);
              progress.list->addItem (tr ("%1 audit point records.").arg (caf_header.number_of_records));
              progress.list->addItem (tr ("%1 points marked invalid by interest point").arg (apply_count - ret_count));
              QListWidgetItem *tot = new QListWidgetItem (tr ("%1 points marked invalid by return").arg (ret_count));

              progress.list->addItem (tot);
              progress.list->setCurrentItem (tot);
              progress.list->scrollToItem (tot);

              apply_count = 0;

              progress.fbar->setValue (100);
            }
          czmil_close_caf_file (caf_hnd);
        }

      progress.obar->setValue (i * 100);
      qApp->processEvents ();
    }


  progress.obar->setValue (input_file_count * 100);


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  progress.list->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Processing complete, press Finish to exit."));

  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);
}



//  Get the users defaults.

void
czmilApplyAudit::envin ()
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options.font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options.font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options.font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options.window_x = 0;
  options.window_y = 0;
  options.window_width = 900;
  options.window_height = 500;
  options.input_dir = ".";
  options.tolerance = 1.0;
  options.return_check = NVFalse;


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/czmilApplyAudit.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/czmilApplyAudit.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("czmilApplyAudit");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options.window_width = settings.value (QString ("width"), options.window_width).toInt ();
  options.window_height = settings.value (QString ("height"), options.window_height).toInt ();
  options.window_x = settings.value (QString ("x position"), options.window_x).toInt ();
  options.window_y = settings.value (QString ("y position"), options.window_y).toInt ();

  options.tolerance = settings.value (QString ("tolerance"), options.tolerance).toFloat ();
  options.return_check = settings.value (QString ("return check"), options.return_check).toBool ();

  settings.endGroup ();
}




//  Save the users defaults.

void
czmilApplyAudit::envout ()
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/czmilApplyAudit.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/czmilApplyAudit.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("czmilApplyAudit");


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("width"), options.window_width);
  settings.setValue (QString ("height"), options.window_height);
  settings.setValue (QString ("x position"), options.window_x);
  settings.setValue (QString ("y position"), options.window_y);

  settings.setValue (QString ("tolerance"), options.tolerance);
  settings.setValue (QString ("return check"), options.return_check);

  settings.endGroup ();
}



void 
czmilApplyAudit::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}
