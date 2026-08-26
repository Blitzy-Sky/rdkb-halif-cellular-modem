/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file cellular_modem_hal_api.h
 * @brief This file defines an interface for controlling and querying the state of cellular modems.
 *
 * The interface provides functions for:
 *
 * - **Modem Management:**
 *    - Factory reset (`Modem_FactoryReset()`)
 *    - Reboot (`Modem_Reboot()`)
 *    - Firmware version retrieval (`Modem_Firmware_Version()`)
 *
 * This header is the complete Cellular Modem HAL contract: the three declarations
 * above and the two status macros below are everything it defines. It declares no
 * enumeration, structure, typedef or context type, no initialization or teardown
 * entry point, and no callback registration, so there is no handle to carry between
 * calls and nothing here delivers an asynchronous notification. The repository
 * specification records the same scope under `Initialization and Startup`,
 * `Asynchronous Notification Model` and `Data Structures and Defines`
 * (`docs/pages/halSpec.md`).
 *
 * Two of the three operations act on the modem - `Modem_FactoryReset()` and
 * `Modem_Reboot()` - and the third, `Modem_Firmware_Version()`, reads a value into a
 * buffer the caller supplies. Nothing in this interface configures the radio,
 * reports IMEI or ICCID identity, or controls a SIM or a power state; those belong to
 * other RDK-B interfaces and are not reachable through this header.
 *
 * Behaviour stated in this header is derived from these declarations and from the
 * repository specification. Nothing is presented as observed runtime behaviour, and
 * where neither source establishes a behaviour the block that needs it says so
 * explicitly rather than assuming one.
 */


#ifndef __CCSP_HAL_CELL_MAN_H__
#define __CCSP_HAL_CELL_MAN_H__

#define STATUS_SUCCESS 0    /*!< Value 0, returned by all three functions when the requested operation completed. On this value, and only on this value, a caller may act on what the call produced: for Modem_Firmware_Version() that means reading the buffer it was given. */
#define STATUS_FAILED -1    /*!< Value -1, returned by all three functions when the requested operation did not complete. It is the interface's only failure value and carries no cause: nothing distinguishes an invalid argument from a hardware failure, so a caller can retry, escalate or report but cannot branch on the reason. */

#ifdef __cplusplus
extern "C"{
#endif


/*
 * Return code contract for the three functions below.
 *
 * Each returns an int and reports its outcome synchronously through that return
 * value; no failure is reported out of band, which the repository specification
 * states under `Internal Error Handling` (`docs/pages/halSpec.md`). STATUS_SUCCESS
 * and STATUS_FAILED, declared above, are the complete return vocabulary this
 * interface defines - it declares no error enumeration, no diagnostic out-parameter
 * and no secondary status call, so every failure arrives as the same undifferentiated
 * value.
 *
 * Each declaration below lists the conditions an implementation may attribute a
 * failure to. Those are descriptive prose rather than distinct codes and cannot be
 * recovered from the return value, so a caller's failure handling is uniform across
 * the interface: it may retry, escalate or report, but it cannot branch on the cause.
 * Diagnosing a cause means reading the vendor log the repository specification
 * describes under `Logging and debugging requirements`.
 */

/**
 * @brief Restores the modem to its factory settings, discarding its custom configuration.
 *
 * Resets the modem to its default factory settings, erasing any custom configurations and restoring all settings to their original state. This is typically used to resolve software issues or to clean the configuration before a new setup.
 *
 * The call is destructive and its effect is on the modem rather than on this
 * interface: after a successful reset the modem's configuration is the factory one,
 * so any setting a caller or an earlier provisioning step had applied to the modem is
 * gone and has to be applied again. The repository specification states under
 * `Persistence Model` that this HAL persists nothing on a caller's behalf, so state a
 * calling application needs to survive the reset is the application's to keep.
 *
 * What a caller can observe afterwards is only the return value. This interface
 * declares no status, readiness or configuration query - the two macros above are its
 * only enumerated values - so a caller cannot confirm the post-reset configuration
 * through this header, and there is no notification either, which the repository
 * specification records under `Asynchronous Notification Model`.
 *
 * @pre None imposed by this interface. It declares no initialization, open or
 *      teardown call and no ordering between its three functions, which the
 *      repository specification states under `Initialization and Startup` and
 *      `Method Sequencing`, so the call is legal at any time and there is no
 *      caller-side setup step whose omission could fail it. A caller does depend on
 *      a vendor implementation of this header being linked and on the modem hardware
 *      being reachable; where the hardware is not ready the call blocks rather than
 *      failing, as the blocking note below records.
 * @post On STATUS_SUCCESS the modem holds its factory configuration. This interface
 *       does not state when the modem is usable again, nor whether it restarts as
 *       part of the reset, so a caller must not treat the return as a readiness
 *       signal.
 * @post On STATUS_FAILED the modem's configuration is unspecified by this interface.
 *       The reset may have been rejected before it began or interrupted part way
 *       through - the single failure code does not distinguish the two - so a caller
 *       must not assume either the previous configuration or the factory one, and must
 *       re-establish the configuration it requires rather than inferring it.
 *
 * @returns Status of the operation:
 * @retval STATUS_SUCCESS - On successful factory reset of the modem. The client
 *                          continues, treating every modem setting it depends on as
 *                          needing to be applied again.
 * @retval STATUS_FAILED - On failure. The client must not assume the modem's
 *                          configuration state; it logs the failure, may retry, and
 *                          escalates a persistent failure to the vendor
 *                          implementation's owner. Retrying cannot be told apart from
 *                          a first attempt by this interface, so a client that must
 *                          not reset twice tracks that itself. Potential failure
 *                          reasons include:
 *                          - Failure to access or modify configuration storage
 *                          - Insufficient permissions to perform the reset
 *                          - Hardware or firmware issues that prevent resetting
 *                          - Interruptions during the reset process
 *                          None of them is separately reportable: they are the
 *                          conditions an implementation may attribute a failure to,
 *                          not distinct return codes.
 *
 * @note Blocking: the call is synchronous and blocks until it has an outcome to
 *       report, and the repository specification states under `Initialization and
 *       Startup` and `Blocking calls` that this interface is designed to block while
 *       the modem hardware is not ready for operation. A caller must not issue it
 *       from a thread that cannot tolerate being suspended, and must not read a slow
 *       return as a failure. This interface specifies no maximum duration and no
 *       timeout value; the real duration of a factory reset is a property of the
 *       modem hardware. Implementations are required to apply a timeout where a
 *       response may be absent, but no numeric bound is stated anywhere in this
 *       repository, so a caller that cannot tolerate an unbounded wait imposes its
 *       own bound. A test author must not assert a completion time.
 * @note This interface does not specify what happens to a call issued while a reset
 *       is in progress, or during the window in which the modem is recovering from
 *       one - whether it fails, blocks or is serviced - and it provides no readiness
 *       signal for the end of that window, as `Method Sequencing` records. A caller
 *       that has to sequence work around a reset takes that signal from the platform
 *       or the vendor, not from this header.
 * @warning Not thread safe. The repository specification states under `Threading
 *          Model` that this interface provides no synchronization and declares no
 *          lock, handle or context through which a caller could scope one, so the
 *          caller serialises this call against every other call into this interface.
 *          Re-entrancy is not claimed for any function here, so serialising only
 *          calls to the same function is not sufficient. The functions are expected
 *          to be callable from multiple processes and `Process Model` places the
 *          obligation to protect concurrent use on the vendor implementation; this
 *          interface does not state what a second caller observes while a reset is in
 *          flight, so a caller must not rely on either serialisation or rejection
 *          across process boundaries.
 * @warning The configuration this call discards is not recoverable through this
 *          interface. There is no read-back and no undo.
 * @see Modem_Reboot
 * @see Modem_Firmware_Version
 */
int Modem_FactoryReset(void);

/**
 * @brief Restarts the modem hardware, taking it down and bringing it back up.
 *
 * Initiates a reboot of the modem hardware. This operation attempts to safely shutdown and restart the modem,
 * ensuring that all services are cleanly restarted. This is typically used to apply new settings or recover from
 * an unstable state.
 *
 * The call is disruptive: the modem stops carrying traffic while it restarts, so any
 * connectivity that depends on it is interrupted for the duration. Unlike
 * Modem_FactoryReset() it does not discard the modem's configuration - it restarts the
 * modem with the configuration it already holds, which is why it is the call used to
 * bring a pending setting into effect or to clear an unstable state.
 *
 * What a caller can observe afterwards is only the return value. Nothing in this
 * interface reports that the restart has finished: it declares no readiness or status
 * query and no callback, which the repository specification records under
 * `Asynchronous Notification Model` and `State Diagram`.
 *
 * @pre None imposed by this interface. It declares no initialization, open or
 *      teardown call and no ordering between its three functions, which the
 *      repository specification states under `Initialization and Startup` and
 *      `Method Sequencing`, so the call is legal at any time and there is no
 *      caller-side setup step whose omission could fail it. A caller does depend on a
 *      vendor implementation of this header being linked and on the modem hardware
 *      being reachable; where the hardware is not ready the call blocks rather than
 *      failing, as the blocking note below records.
 * @post On STATUS_SUCCESS the reboot has been carried out as far as this interface
 *       reports it. This interface does not state whether the return means the restart
 *       was initiated or completed, when the modem is usable again, or how a caller
 *       learns that it is, so the return value must not be treated as a readiness
 *       signal and a caller must not issue dependent work on the strength of it alone.
 * @post On STATUS_FAILED the modem's operational state is unspecified by this
 *       interface. The reboot may have been refused before anything happened or have
 *       failed during the shutdown phase - the single failure code does not
 *       distinguish the two - so a caller must not assume the modem is either running
 *       as before or down.
 *
 * @returns Status of the reboot operation:
 * @retval STATUS_SUCCESS - On successful reboot of the modem. The client continues,
 *                          and obtains the readiness signal it needs for dependent
 *                          work from outside this interface.
 * @retval STATUS_FAILED - On failure. The client must not assume the modem's
 *                          operational state; it logs the failure, may retry, and
 *                          escalates a persistent failure to the vendor
 *                          implementation's owner. Potential failure reasons include:
 *                          - Inability to secure necessary system resources for reboot
 *                          - Failure in initial shutdown phase of the modem
 *                          - Hardware or firmware issues preventing reboot
 *                          - Security or permission restrictions
 *                          None of them is separately reportable: they are the
 *                          conditions an implementation may attribute a failure to,
 *                          not distinct return codes.
 *
 * @note Blocking: the call is synchronous and blocks until it has an outcome to
 *       report, and the repository specification states under `Initialization and
 *       Startup` and `Blocking calls` that this interface is designed to block while
 *       the modem hardware is not ready for operation. A caller must not issue it from
 *       a thread that cannot tolerate being suspended. This interface specifies no
 *       maximum duration and no timeout value, and the real duration of a modem
 *       restart is a property of the hardware; implementations are required to apply a
 *       timeout where a response may be absent, but no numeric bound is stated
 *       anywhere in this repository. A caller that cannot tolerate an unbounded wait
 *       imposes its own bound, and a test author must not assert a completion time.
 * @note This interface does not specify what happens to a call issued while the modem
 *       is restarting - whether it fails, blocks or is serviced - and it provides no
 *       signal for the end of that window, as `Method Sequencing` records. In
 *       particular, calling Modem_Firmware_Version() immediately after this function
 *       is not established to succeed.
 * @warning Not thread safe. The repository specification states under `Threading
 *          Model` that this interface provides no synchronization and declares no
 *          lock, handle or context through which a caller could scope one, so the
 *          caller serialises this call against every other call into this interface.
 *          Re-entrancy is not claimed for any function here, so serialising only calls
 *          to the same function is not sufficient. The functions are expected to be
 *          callable from multiple processes and `Process Model` places the obligation
 *          to protect concurrent use on the vendor implementation; this interface does
 *          not state what a second caller observes while a reboot is in flight, so a
 *          caller must not rely on either serialisation or rejection across process
 *          boundaries.
 * @warning The restart interrupts whatever the modem is carrying. A caller that shares
 *          the modem with other components coordinates with them outside this
 *          interface, because nothing here reports that a reboot is pending or in
 *          progress.
 * @see Modem_FactoryReset
 * @see Modem_Firmware_Version
 */
int Modem_Reboot(void);

/**
 * @brief Reads the modem's current firmware version into a caller-supplied buffer.
 *
 * This function retrieves the modem's firmware version and stores it in the provided buffer.
 *
 * The call is a read: it changes nothing on the modem, and the version it reports is
 * whatever firmware the modem is currently running. The value leaves through the
 * out-parameter while the return value reports only whether the read succeeded, so
 * this function is status-returning rather than value-returning - the distinction the
 * repository specification draws under `API Surface` (`docs/pages/halSpec.md`).
 *
 * After a successful call the caller can observe the version characters in its own
 * buffer, subject to the bounds below. Nothing else about the modem becomes observable:
 * this interface reports no build date, no component breakdown and no upgrade state.
 *
 * @param[out] firmware_version - Buffer (min. 128 bytes) to store the firmware version (e.g., "EM06ALAR04A01M4G_01.003.01.003").
 *                          \n Capacity: the caller must supply writable storage of at
 *                          least 128 bytes. That minimum is the only bound this
 *                          interface states, and no argument carries the capacity the
 *                          caller actually allocated, so an implementation cannot
 *                          learn of a smaller buffer, cannot report truncation, and
 *                          must confine its write to 128 bytes; a caller must not
 *                          supply less.
 *                          \n Ownership and retention: the caller allocates the buffer
 *                          before the call and retains ownership throughout, and it is
 *                          the caller that eventually releases the storage - though not
 *                          on the strength of this call returning, for the reason
 *                          below. This interface never frees caller memory and no
 *                          declaration here transfers ownership. Whether an
 *                          implementation retains the pointer beyond the call is not
 *                          specified by this interface: the declaration does not state
 *                          a lifetime for it, and the absence of a callback is not
 *                          evidence that no reference is kept. A caller must therefore
 *                          not treat the return as the point at which the reference is
 *                          dropped; it keeps the buffer allocated after the call and
 *                          establishes with the vendor implementation whether the
 *                          storage may be released or reused before doing either. The
 *                          repository specification records the same absence under
 *                          `Memory Model`.
 *                          \n Must not be NULL. This interface does not oblige an
 *                          implementation to detect a bad pointer and, having one
 *                          failure value that carries no cause, could not report the
 *                          detection distinguishably if it did: passing NULL is a
 *                          caller defect, not a supported way to query anything, and a
 *                          caller must not read a failure as confirmation that its
 *                          pointer was the problem.
 *                          \n Termination: **this interface does not state that the
 *                          value written is NUL-terminated**, does not state how many
 *                          bytes are written, and does not state whether anything is
 *                          written when the call fails. A caller must therefore not
 *                          treat the buffer as a C string on the strength of this
 *                          interface alone. The safe reading is to zero the whole
 *                          buffer before the call and to bound every subsequent read
 *                          by the size allocated rather than by a terminator - for
 *                          example with a length-bounded scan over that size instead
 *                          of a function that scans until a NUL. A caller that needs a
 *                          guaranteed C string allocates more than the 128-byte
 *                          minimum and zeroes it, so that even a write filling all 128
 *                          bytes leaves a terminating zero byte inside its own
 *                          storage; that guarantee comes from the caller's own
 *                          allocation and initialisation, never from this interface.
 *
 * @pre None imposed by this interface beyond the buffer. It declares no
 *      initialization, open or teardown call and no ordering between its three
 *      functions, which the repository specification states under `Initialization and
 *      Startup` and `Method Sequencing`, so the call is legal at any time. The one
 *      pre-condition is that `firmware_version` addresses at least 128 writable bytes
 *      the caller owns; if it does not, the behaviour is not defined by this interface
 *      - an implementation may return STATUS_FAILED, but a buffer that is too small or
 *      not writable can equally be overrun or fault, because no argument lets an
 *      implementation discover either condition.
 * @post On STATUS_SUCCESS the buffer holds the firmware version, read within the
 *       bounds and termination rules above.
 * @post On STATUS_FAILED the buffer content is unspecified by this interface. It may
 *       be untouched or partially written, so a caller must discard it rather than
 *       read it, and must not treat a buffer it zeroed before the call as evidence
 *       that nothing was written.
 *
 * @returns Status of the operation:
 * @retval STATUS_SUCCESS - On success. The client reads the version from its buffer,
 *                          bounding the read as described above.
 * @retval STATUS_FAILED - On failure (e.g., invalid parameter, retrieval error). The
 *                          client discards the buffer content, logs the failure and
 *                          may retry; a caller-side defect such as a NULL or
 *                          undersized buffer cannot be told apart from a modem read
 *                          failure through this interface, so a client that keeps
 *                          failing checks its own call site as well as escalating to
 *                          the vendor implementation's owner.
 *
 * @note Blocking: the call is synchronous and blocks until the version has been read
 *       or the read has failed, and the repository specification states under
 *       `Initialization and Startup` and `Blocking calls` that this interface is
 *       designed to block while the modem hardware is not ready for operation. This
 *       interface specifies no maximum duration and no timeout value; implementations
 *       are required to apply a timeout where a response may be absent, but no numeric
 *       bound is stated anywhere in this repository, so a caller that cannot tolerate
 *       an unbounded wait imposes its own bound.
 * @note This interface does not establish that the call succeeds while the modem is
 *       restarting after Modem_Reboot() or resetting after Modem_FactoryReset(), and it
 *       provides no readiness signal for either window, as `Method Sequencing` records.
 * @warning Not thread safe. The repository specification states under `Threading
 *          Model` that this interface provides no synchronization and declares no lock,
 *          handle or context through which a caller could scope one, so the caller
 *          serialises this call against every other call into this interface, and
 *          re-entrancy is not claimed for any function here. The functions are expected
 *          to be callable from multiple processes and `Process Model` places the
 *          obligation to protect concurrent use on the vendor implementation. A caller
 *          must additionally not let two of its own threads share one buffer across
 *          concurrent calls: nothing in this interface serialises writes into it.
 * @warning Because termination is not established, passing this buffer to a routine
 *          that scans for a terminator - or logging it as a string - can read beyond
 *          the storage the caller allocated. Bound every read by the allocated size.
 * @see Modem_FactoryReset
 * @see Modem_Reboot
 */
int Modem_Firmware_Version(char *firmware_version);

#ifdef __cplusplus
}
#endif

#endif
