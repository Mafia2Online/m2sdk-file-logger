class M2ModModule : public M2::C_TickedModule {
public:
    void init(M2::I_TickedModuleCallEventContext &);
    void tick(M2::I_TickedModuleCallEventContext &);
    void load_start(M2::I_TickedModuleCallEventContext &);
    void load_finish(M2::I_TickedModuleCallEventContext &);
};

static M2ModModule m2mod;

void mod_install() {
    mod_log("[info] mod_init\n");

    auto tmm = M2::C_TickedModuleManager::Get();
    using tc = void (M2::C_TickedModule::*)(M2::I_TickedModuleCallEventContext &);

    tmm->AddAction(M2::TickedModuleEvent::EVENT_GAME_INIT,        500,  &m2mod, (tc)(&M2ModModule::init), -1.0f, 0, 0);
    tmm->AddAction(M2::TickedModuleEvent::EVENT_TICK,             400,  &m2mod, (tc)(&M2ModModule::tick), -1.0f, 0, 0);
    tmm->AddAction(M2::TickedModuleEvent::EVENT_LOADING_STARTED,  1720, &m2mod, (tc)(&M2ModModule::load_start), -1.0f, 0, 0);
    tmm->AddAction(M2::TickedModuleEvent::EVENT_LOADING_FINISHED, 1720, &m2mod, (tc)(&M2ModModule::load_finish), -1.0f, 0, 0);

    M2::AttachHandler(M2_EVENT_MOD_MESSAGE, [](m2sdk_event *data) {
        auto message = (int)data->arg1;

        switch (message) {
            case M2::E_PlayerMessage::MESSAGE_MOD_ENTER_CAR: {
                mod_log("[game-event] ped start to enter vehicle\n");
            } break;

            case M2::E_PlayerMessage::MESSAGE_MOD_BREAKIN_CAR: {
                mod_log("[game-event] Start to breakin vehicle\n");
            } break;

            case M2::E_PlayerMessage::MESSAGE_MOD_LEAVE_CAR: {
                mod_log("[game-event] Start to leave car\n");
            } break;
        }
    });

    M2::AttachHandler(M2_EVENT_GAME_MESSAGE, [](m2sdk_event *data) {
        auto message = (M2::C_EntityMessage *)data->arg1;

        switch (message->m_dwMessage) {
            case M2::E_HumanMessage::MESSAGE_GAME_ENTER_EXIT_VEHICLE_DONE: {
                mod_log("[game-event] Enter/Exit Vehicle done\n");
            } break;

            default: {
                mod_log("[game-event] unknown event %d\n", (int)message->m_dwMessage);
            } break;
        }
    });

    M2::AttachHandler(M2_EVENT_CAR_ENTER_REQUEST, [](m2sdk_event *data) {
        data->arg5 = (void *)true;
        // data->arg5 = (void *)false; // to block entering the car

        mod_log("[game-event] ped request vehicle enter (%s)\n", ((bool)data->arg5) ? "granted" : "denied");
    });

    M2::AttachHandler(M2_EVENT_CAR_ENTER, [](m2sdk_event *data) {
        auto player = (M2::C_Player2 *)data->arg1;
        auto car    = (M2::C_Car *)data->arg2;
        auto seat   = (int)data->arg3;

        mod_log("[game-event] ped entering the car on seat: %d\n", seat);
    });
}
void mod_respawn() {
    M2::C_GfxEnvironmentEffects::Get()->GetWeatherManager()->SetTime(0.5); /* 0.0 .. 1.0 - time of the day */

    auto ped = (M2::C_Entity *)M2::C_Game::Get()->GetLocalPed();

    if (M2::C_SDSLoadingTable::Get()) {
        M2::C_SDSLoadingTable::Get()->ProcessLine("free_joe_load");
        M2::C_SDSLoadingTable::Get()->ProcessLine("free_summer_load");

        M2::C_GfxEnvironmentEffects::Get()->GetWeatherManager()->SetDayTemplate("DT_RTRclear_day_late_afternoon");
        mod_log("[info] setting day template: %s\n", "DT_RTRclear_day_late_afternoon");
    }

    /* Disable ambiant peds */
    M2::Wrappers::SwitchFarAmbiants(false);
    M2::Wrappers::SwitchGenerators(false);

    /* Lock to prevent actions while respawning */
    ((M2::C_Player2 *)ped)->LockControls(true);

    /* Resetting player */
    ((M2::C_Human2 *)ped)->GetScript()->SetHealth(720.0f);
    ((M2::C_Entity *)ped)->SetPosition(Vector3(-421.75f, 479.31f, 0.05f));

    /* Enabling controls */
    ((M2::C_Player2 *)ped)->LockControls(false);
}

// =======================================================================//
// !
// ! General mod events
// !
// =======================================================================//

void M2ModModule::init(M2::I_TickedModuleCallEventContext &) {
    mod_log("[GameModule]: EventGameInit\n");

    // therotically we shouldn't call it here but because it's a
    // sync object it's fine itll work but the local player isn't created just yet.
    M2::C_GameGuiModule::Get()->FaderFadeIn(1);
}

void M2ModModule::load_start(M2::I_TickedModuleCallEventContext &) {
    mod_log("[GameModule]: EventLoadingStarted\n");
}

void M2ModModule::load_finish(M2::I_TickedModuleCallEventContext &) {
    mod_log("[GameModule]: EventLoadingFinished\n");
    std::thread([]() {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1500)
        );

        mod_respawn();
    }).detach();
}

void M2ModModule::tick(M2::I_TickedModuleCallEventContext &) {
    static M2::C_Entity *ent;
    if (GetAsyncKeyState(VK_F3) & 0x1) {
        ent = M2::Wrappers::CreateEntity(M2::eEntityType::MOD_ENTITY_CAR, 10);
        auto pos = reinterpret_cast<M2::C_Human2*>(M2::C_Game::Get()->GetLocalPed())->GetPos();
        ent->SetPosition(pos);
        mod_log("Ped created\n");
    }

    if (GetAsyncKeyState(VK_F4) & 0x1 && ent) {
        Vector3 dir = reinterpret_cast<M2::C_Human2*>(M2::C_Game::Get()->GetLocalPed())->GetDir();
        M2::S_HumanCommandMoveDir *moveCMD = new M2::S_HumanCommandMoveDir;
        moveCMD->x = dir.x;
        moveCMD->y = dir.y;
        moveCMD->z = dir.z;
        reinterpret_cast<M2::C_Human2*>(ent)->AddCommand(M2::E_Command::COMMAND_MOVEDIR, moveCMD);

        mod_log("Command added\n");
    }

    // if (input_key_down(VK_F6) && mod.spawned) {
    //     void *command;
    //     void *command2;
    //     command2 =  reinterpret_cast<M2::C_Human2*>(M2::C_Game::Get()->GetLocalPed())->GetCurrentMoveCommand(&command);
    //     mod_log("0x%p\n0x%p\n", command, command2);
    // }

    if (GetAsyncKeyState(VK_F7) & 0x1) {
        reinterpret_cast<M2::C_Human2*>(M2::C_Game::Get()->GetLocalPed())->m_pCurrentCar->SetVehicleDirty(100.0);
    }

    if (GetAsyncKeyState(VK_F6) & 0x1) {
        M2::C_Car *car = reinterpret_cast<M2::C_Human2*>(M2::C_Game::Get()->GetLocalPed())->m_pCurrentCar;
        if (!car) {
            mod_log("null ptr\n");
            return;
        }

        float level = *(float*)(car + 0xE8C);
        mod_log("level : %d\n", level);
    }
}
