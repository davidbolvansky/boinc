// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// If you change anything, make sure you also change:
// client_types.C         (to write and parse it)
// client_state.C  (to cross-link objects)
//

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#include "cpp.h"

#ifndef _WIN32
#include <stdio.h>
#include <sys/time.h>
#endif

#include "md5_file.h"
#include "hostinfo.h"
#include "miofile.h"
#include "result_state.h"

#define MAX_BLOB_LEN 4096
#define P_LOW 1
#define P_MEDIUM 3
#define P_HIGH 5

struct STRING256 {
    char text[256];
};

// If the status is neither of these two,
// it will be an error code defined in error_numbers.h,
// indicating an unrecoverable error in the upload or download of the file,
// or that the file was too big and was deleted
//
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1

class FILE_INFO {
public:
    char name[256];
    char md5_cksum[33];
    double max_nbytes;
    double nbytes;
    double upload_offset;
    bool generated_locally; // file is produced by app
    int status;
    bool executable;        // change file protections to make executable
    bool uploaded;          // file has been uploaded
    bool upload_when_present;
    bool sticky;            // don't delete unless instructed to do so
    bool report_on_rpc;     // include this in each scheduler request
    bool signature_required;    // true iff associated with app version
    bool is_user_file;
    class PERS_FILE_XFER* pers_file_xfer;   // nonzero if in the process of being up/downloaded
    struct RESULT* result;         // for upload files (to authenticate)
    class PROJECT* project;
    int ref_cnt;
    std::vector<STRING256> urls;
    int start_url;
    int current_url;
    char signed_xml[MAX_BLOB_LEN];
        // if the file_info is signed (for uploadable files)
        // this is the text that is signed
    char xml_signature[MAX_BLOB_LEN];
        // ... and this is the signature
    char file_signature[MAX_BLOB_LEN];
        // if the file itself is signed (for executable files)
        // this is the signature
#if 0
    int priority;
    time_t time_last_used;         // time of last use of FILE_INFO, update during parsing, writing, or application usage
    time_t exp_date;
#endif
    std::string error_msg;       // if permanent error occurs during file xfer,
                            // it's recorded here

    FILE_INFO();
    ~FILE_INFO();
    void reset();
    int set_permissions();
    int parse(MIOFILE&, bool from_server);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    int delete_file();      // attempt to delete the underlying file
    char* get_init_url(bool);
    char* get_next_url(bool);
    char* get_current_url(bool);
    bool is_correct_url_type(bool, STRING256);
    bool had_failure(int& failnum, char* buf=0);
    bool verify_existing_file();
    int merge_info(FILE_INFO&);
    int verify_downloaded_file();
    int update_time();       // updates time last used to the current time
};

// Describes a connection between a file and a workunit, result, or application.
// In the first two cases,
// the app will either use open() or fopen() to access the file
// (in which case "open_name" is the name it will use)
// or the app will be connected by the given fd (in which case fd is nonzero)
//
struct FILE_REF {
    char file_name[256];
    char open_name[256];
    int fd;
    bool main_program;
    FILE_INFO* file_info;
    bool copy_file;  // if true, core client will copy the file instead of linking

    int parse(MIOFILE&);
    int write(MIOFILE&);
};

class PROJECT {
public:
    // the following items come from the account file
    // They are a function only of the user and the project
    //
    char master_url[256];       // url of site that contains scheduler tags
                                // for this project
    char authenticator[256];    // user's authenticator on this project
#if 0
                                        // deletion policy, least recently used
    bool deletion_policy_priority;       // deletion policy, priority of files
    bool deletion_policy_expire;         // deletion policy, delete expired files first
    double share_size;          // size allocated by the resource share
                                // used for enforcement of boundaries but isn't one itself
    double size;                // the total size of all the files in all subfolder
                                // of the project
#endif
    std::string project_prefs;
        // without the enclosing <project_preferences> tags.
        // May include <venue> elements
        // This field is used only briefly: between handling a
        // scheduler RPC reply and writing the account file
    std::string project_specific_prefs;
        // without enclosing <project_specific> tags
        // Does not include <venue> elements
    std::string gui_urls;
        // GUI URLs, with enclosing <gui_urls> tags
    double resource_share;
        // project's resource share relative to other projects.
    char host_venue[256];

    // the following items come from client_state.xml
    // They may depend on the host as well as user and project
    // NOTE: if you add anything, add it to copy_state_fields() also!!!
    //
    std::vector<STRING256> scheduler_urls;       // where to find scheduling servers
    char project_name[256];             // descriptive.  not unique
    char user_name[256];
    char team_name[256];
    char email_hash[MD5_LEN];
    char cross_project_id[MD5_LEN];
    double user_total_credit;    // as reported by server
    double user_expavg_credit;    // as reported by server
    unsigned int user_create_time;   // as reported by server
    int rpc_seqno;
    int hostid;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    unsigned int host_create_time; // as reported by server
    double exp_avg_cpu;            // exponentially weighted CPU time
    double exp_avg_mod_time;       // last time average was changed
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    time_t min_rpc_time;           // earliest time to contact any server
                                  // of this project (or zero)
    time_t min_report_min_rpc_time; // when to next report on min_rpc_time
                                    // (or zero)
    bool master_url_fetch_pending;
                                // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed
    bool anonymous_platform;    // app_versions.xml file found in project dir;
                                // use those apps rather then getting from server
    bool non_cpu_intensive;
    bool send_file_list;
        // send the list of permanent files associated/with the project
        // in the next scheduler reply
    bool suspended_via_gui;

    char code_sign_key[MAX_BLOB_LEN];
    std::vector<FILE_REF> user_files;
    int parse_preferences_for_user_files();
    
    // the following fields used by CPU scheduler
    double debt;                // how much CPU time we owe this project (secs)

    // the following items are transient; not saved in state file
    double anticipated_debt;    // expected debt by the end of the preemption period
    double work_done_this_period; // how much CPU time has been devoted to this
                                  // project in the current period (secs)
    struct RESULT *next_runnable_result; // the next result to run for this project
    
    // the following used by work-fetch algorithm
    double work_request;        // how much work a project needs (secs)

#if 0
    // used in disk-space management (temp)
    bool checked;
#endif

    PROJECT();
    ~PROJECT();
    void init();
    void copy_state_fields(PROJECT&);
    char *get_project_name();
    int write_account_file();
    int parse_account(FILE*);
    int parse_account_file();
    int parse_state(MIOFILE&);
    int write_state(MIOFILE&, bool gui_rpc=false);
#if 0
    bool associate_file(FILE_INFO*);
#endif

    // set min_rpc_time and have_reported_min_rpc_time
    void set_min_rpc_time(time_t future_time);
    // returns true if min_rpc_time > now; may print a message
    bool waiting_until_min_rpc_time(time_t now);
};

struct APP {
    char name[256];
    PROJECT* project;

    int parse(MIOFILE&);
    int write(MIOFILE&);
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;

    int parse(MIOFILE&);
    int write(MIOFILE&);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;
        // This isn't sent from the server.
        // Instead, the client picks the latest app version
    char command_line[256];
    char env_vars[256];         // environment vars in URL format
    std::vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;
    int ref_cnt;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;

    int parse(MIOFILE&);
    int write(MIOFILE&);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
};

struct RESULT {
    char name[256];
    char wu_name[256];
    int report_deadline;
    std::vector<FILE_REF> output_files;
    bool ready_to_report;
        // we're ready to report this result to the server;
        // either computation is done and all the files have been uploaded
        // or there was an error
    bool got_server_ack;
        // we're received the ack for this result from the server
    double final_cpu_time;
    int state;              // state of this result: see lib/result_state.h
    int exit_status;        // return value from the application
    std::string stderr_out;
        // the concatenation of:
        //
        // - if report_result_error() is called for this result:
        //   <message>x</message>
        //   <exit_status>x</exit_status>
        //   <signal>x</signal>
        //   - if called in FILES_DOWNLOADED state:
        //     <couldnt_start>x</couldnt_start>
        //   - if called in NEW state:
        //     <download_error>x</download_error> for each failed download
        //   - if called in COMPUTE_DONE state:
        //     <upload_error>x</upload_error> for each failed upload
        //
        // - <stderr_txt>X</stderr_txt>, where X is the app's stderr output

    APP* app;
    WORKUNIT* wup;
        // this may be NULL after result is finished
    PROJECT* project;

    bool already_selected;
        // used to keep cpu scheduler from scheduling a result twice
        // transient; used only within schedule_cpus()
    void clear();
    int parse_server(MIOFILE&);
    int parse_state(MIOFILE&);
    int parse_ack(FILE*);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    bool is_upload_done();    // files uploaded?
    void get_app_version_string(std::string&);
    void reset_files();
};

int verify_downloaded_file(char* pathname, FILE_INFO& file_info);

#endif
