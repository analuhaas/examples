Welcome to my version of the OwnTech examples repository. Here I added some extra examples concerning the MMC converter, communications and DC DC topologies.

Here you can find ready to be used example for [SPIN](https://github.com/owntech-foundation/SPIN) and [TWIST](https://github.com/owntech-foundation/TWIST).

# Examples for TWIST

- DC DC topology examples
    - [Buck voltage mode](TWIST/DC_DC/buck_voltage_mode/README.md)
    - [Buck current mode](TWIST/DC_DC/buck_current_mode/README.md)
    - [Buck voltage mode - typed reference](TWIST/DC_DC/buck_voltage_mode_typed/README.md)
    - [Buck current mode - typed reference](TWIST/DC_DC/buck_current_mode_typed/README.md)
    - [Boost voltage mode](TWIST/DC_DC/boost_voltage_mode/README.md)
    - [Interleaved Buck](TWIST/DC_DC/interleaved/README.md)
    - [Current ripple measure in buck voltage mode](TWIST/DC_DC/current_ripple_measurement/README.md)

- Microgrid examples
    - [AC client server](TWIST/Microgrid/AC_client_server/README.md)
    - [AC peer to peer](TWIST/Microgrid/AC_peer_to_peer/README.md)
    - [DC client server](TWIST/Microgrid/DC_client_server/README.md)
    - [DC droop](TWIST/Microgrid/DC_droop/README.md)

 - DC AC topology examples
    - [Grid forming](TWIST/DC_AC/grid_forming/README.md)
    - [Grid following](TWIST/DC_AC/grid_following/README.md)

# Examples for the MMC

The MMC examples use TWIST boards as MMC modules. They come from the [MMC repository](https://github.com/analuhaas/MMC), where you can find the full documentation and models. In PlatformIO, they appear in the `Examples MMC` and `Examples MMC JCGE` folders of the Project Tasks tab.

- MMC educational prototype (`Examples MMC`)
    - [Manual module calibration](MMC/Educational_prototype/module_calibration/README.md)
    - [Automated module calibration](MMC/Educational_prototype/module_calibration_automated/README.md)
    - [Module test sequences (LF and HF)](MMC/Educational_prototype/module_test_sequences/README.md)
    - [Stack without CVB](MMC/Educational_prototype/stack_no_CVB/README.md)
    - [Stack with CVB](MMC/Educational_prototype/stack_with_cvb/README.md)
    - [Phase with NLM and CVB](MMC/Educational_prototype/phase_NLM_with_cvb/README.md)
    - [CVB control logic](MMC/Control_logic/README.md)
    - [Blinky MMC stack](MMC/Other/blinky_mmc_stack/README.md)
    - [ScopeMimicry plot scripts](MMC/Other/scopemimicry_plot/README.md)

- MMC JCGE, 3rd OwnTech hackathon (`Examples MMC JCGE`)
    - [TWIST modifications for the MMC module](MMC_JCGE/twist_modifications_MMC_module/twist_modifications_MMC_JCGE_tutorial.md)
    - [Manual module calibration](MMC_JCGE/module_calibration/README.md)
    - [Module LF test](MMC_JCGE/mmc_module_lf_test_modified/README.md)
    - [Module HF test](MMC_JCGE/mmc_module_hf_test_modified/README.md)
    - [Stack without CVB](MMC_JCGE/arm_without_cvb/README.md)
    - [Stack with CVB](MMC_JCGE/arm_with_cvb_09dutycycle/README.md)
    - [Phase with CVB](MMC_JCGE/phase_with_cvb_09dutycycle/README.md)

# Examples for SPIN

- ADC use examples
    - [Software trigerred ADC](SPIN/ADC/adc_software_trigger/README.md)
    - [HRTIM trigerred ADC](SPIN/ADC/adc_hrtim_trigger/README.md)

- DAC use examples
    - [Signal generation](SPIN/DAC/signal_generation/README.md)

- LED use examples
    - [Blinky](SPIN/LED/blinky/README.md)

- PWM use examples
    - [PWM duty cycle control](SPIN/PWM/duty_cycle_setting/README.md)
    - [PWM phase shift control](SPIN/PWM/phase_shift/README.md)
    - [Multiple PWM operation](SPIN/PWM/multiple_pwm/README.md)

- TIMER
    - [Incremental encoder](SPIN/TIMER/incremental_encoder/README.md)
